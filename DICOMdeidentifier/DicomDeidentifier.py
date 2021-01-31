# ===============================================================================
# MIT License
#
# Copyright (c) 2019 Kay Sun, Ph.D. (kaysunphd@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ===============================================================================

import os
import sys
import csv
import hmac
import hashlib
import subprocess
import pydicom

in_dir = sys.argv[1]
root_dir = os.path.dirname(in_dir)

hash_me = False
if len(sys.argv) == 3:
    key = sys.argv[2]
    dk = hashlib.pbkdf2_hmac('sha256', bytes(key, 'latin-1'), b'salt', 100000)
    hash_me = True


def hash(data):
    data_bytes = bytes(data, 'latin-1')
    return hmac.new(dk, data_bytes, hashlib.sha256).hexdigest()


def get_tags_for_deid():
    tags_dict = {'removal': [], 'hash': [], 'seq_removal': [], 'seq_hash': []}
    dict_key = ''
    list = []
    read_file = 'DICOM_tags.txt'
    with open(read_file, "r") as f:
        for line in f:
            if line.startswith('##'):
                if dict_key in tags_dict.keys():
                    tags_dict[dict_key].append(list)
                    list = []
            elif "#removal" in line:
                dict_key = "removal"
            elif "#hashed" in line:
                dict_key = "hash"
            elif "#sequence_hash" in line:
                dict_key = "seq_hash"
            elif "#sequence_remove" in line:
                dict_key = "seq_removal"
            elif not line.startswith('#'):
                line = line.strip("\r\n")
                tag = line.split()[0]
                list.append(tag)

    for key, value in tags_dict.items():
        tags_dict[key] = [item for sublist in value for item in sublist]

    return tags_dict


def recursely_remove_select_sequence(ds, removal_list):
    for elem in ds:
        if elem.VR == 'SQ':
            [recursely_remove_select_sequence(item) for item in elem]
        else:
            for remove_tag in removal_list:
                if remove_tag.split(',')[0] == str(elem.tag).split(',')[0] and \
                        remove_tag.split(',')[-1] == str(elem.tag).split(', ')[-1]:
                    remove_tags(ds, elem.tag)


def recursely_remove_sequence(ds):
    for elem in ds:
        if elem.VR == 'SQ':
            [recursely_remove_sequence(item) for item in elem]
        else:
            remove_tags(ds, elem.tag)


def recursely_hash_sequence(ds, removal_list):
    for elem in ds:
        if elem.VR == 'SQ':
            [recursely_hash_sequence(item, removal_list) for item in elem]
        else:
            if elem.VR == 'UI':
                ds.add_new(elem.tag, ds[elem.tag].VR, hash(ds[elem.tag].value))

            for remove_tag in removal_list:
                if remove_tag.split(',')[0] == str(elem.tag).split(',')[0] and \
                        remove_tag.split(',')[-1] == str(elem.tag).split(', ')[-1]:
                    remove_tags(ds, elem.tag)


def remove_tags(ds, dicom_tag):
    if isinstance(ds[dicom_tag].value, float) or isinstance(ds[dicom_tag].value, int) \
            or ds[dicom_tag].VR == 'CS' or ds[dicom_tag].VR == 'IS' or ds[dicom_tag].VR == 'FL'\
            or ds[dicom_tag].VR == 'DS':
        ds.add_new(dicom_tag, ds[dicom_tag].VR, '')
    else:
        if ds[dicom_tag].VR == 'UN':
            ds.add_new(dicom_tag, ds[dicom_tag].VR, b'\x00\x00\x00')
        else:
            ds.add_new(dicom_tag, ds[dicom_tag].VR, 'removed')


def set_output_dir_file(hash_me, caseID, filename):
    is_deided, deided_file_folder = check_if_already_done(hash_me, caseID)
    if hash_me:
        new_filename = filename.split('.')[0] + '.' + hash(".".join(filename.split('.')[1:-1])) + '.dcm'
    else:
        new_filename = filename

    deided_filepath = os.path.join(deided_file_folder, new_filename)

    return deided_filepath


def check_if_already_done(hash_me, caseID):
    if hash_me:
        new_caseid = hash(caseID)
    else:
        new_caseid = caseID

    deided_file_folder = os.path.join(root_dir, 'dicom_deided')
    deided_file_folder = os.path.join(deided_file_folder, new_caseid)
    already_deided = False

    if not os.path.exists(deided_file_folder):
        os.makedirs(deided_file_folder)
    else:
        already_deided = True

    return already_deided, deided_file_folder


if __name__ == "__main__":
    if os.path.isfile("error.log"):
        logger = open("error.log", "a")
    else:
        logger = open("error.log", "w")

    tags_dict = get_tags_for_deid()

    for root, dirs, files in os.walk(in_dir):
        if root == in_dir:
            continue
        caseID = os.path.basename(root)
        print(caseID)
        already_deided, deided_file_folder = check_if_already_done(hash_me, caseID)
        if already_deided:
            print("already anonymized, skipping: ", caseID)
            continue

        for name in files:
            if name.endswith('.dcm'):
                filepath = os.path.join(root, name)

                try:
                    ds = pydicom.dcmread(filepath)
                    print("anonymizing ", filepath)
                    ds.remove_private_tags()
                    deided_filepath = set_output_dir_file(hash_me, caseID, name)

                    for key, value in tags_dict.items():
                        for dicom_tag in value:
                            dicom_tag = dicom_tag[dicom_tag.find("(") + 1:dicom_tag.find(")")]
                            dicom_tag = '0x' + dicom_tag
                            dicom_tag = dicom_tag.replace(',', '')

                            if dicom_tag in ds:
                                tag = pydicom.tag.Tag(dicom_tag)

                                if key == "removal":
                                    remove_tags(ds, dicom_tag)
                                elif key == "hash":
                                    if hash_me:
                                        ds[dicom_tag].value = hash(ds[dicom_tag].value)
                                elif key == "seq_removal":
                                    for seg_ds in ds[dicom_tag].value:
                                        try:
                                            recursely_remove_sequence(seg_ds)
                                        except Exception as e:
                                            print("sequence removal failed with ", filepath)
                                            logger.write("sequence removal failed: {0}\n".format(filepath))
                                            logger.write("\t {0}\n".format(str(e)))
                                elif key == "seq_hash":
                                    if hash_me:
                                        for seg_ds in ds[dicom_tag].value:
                                            try:
                                                recursely_hash_sequence(seg_ds, tags_dict['removal'])
                                            except Exception as e:
                                                print("sequence hash failed with ", filepath)
                                                logger.write("sequence hash failed: {0}\n".format(filepath))
                                                logger.write("\t {0}\n".format(str(e)))

                    try:
                        ds.save_as(deided_filepath)
                    except Exception as e:
                        print("writing dicom failed with ", filepath)
                        logger.write("writing dicom failed {0}\n".format(filepath))
                        logger.write("\t {0}\n".format(str(e)))

                except Exception as e:
                    print("reading dicom failed with ", filepath)
                    logger.write("reading dicom failed {0}\n".format(filepath))
                    logger.write("\t {0}\n".format(str(e)))

    logger.close()