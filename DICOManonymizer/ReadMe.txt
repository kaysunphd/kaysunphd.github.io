October 30 2019
de-identification of DICOMs
------------------------------
Install:
pip install pydicom

How-To-Use:
python DicomAnonymizer.py {path to DICOMs} {optional: secret key password if hashing patient ID}

Example:
if no hash
python DicomAnonymizer.py "C:\Dicom"

if hash
python DicomAnonymizer.py "C:\Dicom" 123abc

DICOM file structure:
DICOM path
    |---- patientID
              |---- *.DCM
              |---- *.DCM
              ...
    |---- patientID
              |---- *.DCM
              |---- *.DCM
              ...
              
Modify:
change the list of dicom tags in DICOM_tags.txt
