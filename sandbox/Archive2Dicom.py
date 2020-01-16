#!/usr/bin/python


##############################################################################
#      TITLE:  Archive2Dicom
##############################################################################

import glob
import os
import sys
import datetime
import shutil
import json
import subprocess
import re
import signal
from ScanPatientDB import DBScanner

##############################################################################
#      Class:  AppConfig
#############################################################################


class AppConfig:
    def __init__(self, fileSpec):
        f = open(fileSpec)
        self.config = json.loads(f.read())
        f.close();

    def get(self, target):
        return self.config[target]


##############################################################################
#      Class:  ArchiveTool
##############################################################################


class ArchiveTool:
    def __init__(self, fileSpec):
        userhomedir = os.getenv('HOME')
        self.tempDirectory = userhomedir + "/ExtractedArchiveData"
        try:
            if os.path.exists(self.tempDirectory):
                shutil.rmtree(self.tempDirectory)

            os.mkdir(self.tempDirectory)
        except:
            print("Directory create failure: ", self.tempDirectory)
            exit()
        self.extractArchive(fileSpec)
        
    # Extract Archive
    def extractArchive(self, fileSpec):
        os.system("gtar -xf " + fileSpec + " -C " + self.tempDirectory)

    # Remove Temp Directory
    def clearUp(self):
        shutil.rmtree(self.tempDirectory)


################################################################################
def get_highest_version_folder(base_path, folder):
    """
    Returns the highest version numbered folder in the given path
    :param base_path: The path to search through
    :param folder: the main folder name you are looking for
    :return: The highest-version numbered folder
    """
    path_string = os.path.join(base_path, folder + '*')
    files = glob.glob(path_string)

    highest_base_num = -1
    highest_version_folder = ''
    for item in files:
        if os.path.isdir(item):
            split_base = os.path.basename(item).split('_')
            if len(split_base) != 2:
                continue
            try:
                base_num = float(split_base[1])
            except ValueError:
                continue
            if base_num > highest_base_num:
                highest_base_num = base_num
                highest_version_folder = item
    return highest_version_folder


################################################################################
def set_vars():
    """
    Description: function to set all the environment variables to launch pinnacle from Command line

    Args: NA.

    Returns: NA

    """

    home_dir = '/usr/local/adacnew'
    os.environ['ROOTDIR'] = home_dir
    os.environ['PINN_ROOT'] = home_dir

    # home_dir = os.getenv('HOME')
    # # pinn_dir = '/home/clp/i386'
    # os.environ['ROOTDIR'] = home_dir+'/work'
    # os.environ['PINN_ROOT'] = home_dir+'/work/Pinnacle'

    os.environ['PATIENTS'] = home_dir + '/Patients'
    os.environ['PINNLP_DATASETS'] = os.getenv('PINN_ROOT') + '/DataSets'

    os.environ['PINN_STATIC'] = os.getenv('PINN_ROOT') + '/PinnacleStatic'
    os.environ['PINN_SITE'] = os.getenv('PINN_ROOT') + '/PinnacleSiteData/'
    os.environ['PINNLP_PATIENTS'] = os.getenv('PATIENTS')
    # Get the architecture
    proc = subprocess.Popen([os.getenv('PINN_STATIC') + '/bin/common/architecture'], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out, err = proc.communicate()
    arch = out.decode('UTF-8')
    arch = os.linesep.join([s for s in arch.splitlines() if s])
    os.environ['PINNACLE_EXEC'] = os.getenv('PINN_STATIC') + '/bin/' + arch + '/Pinnacle'
    os.environ['LIBDIR'] = os.getenv('PINN_ROOT')
    os.environ['PINNLP_STATIC'] = os.getenv('PINN_ROOT') + '/LPStatic'

    base_plugin_static_folder = os.path.join(os.getenv('PINN_ROOT'), 'PluginStatic')
    versioned_plugin_static_folder = get_highest_version_folder(os.getenv('PINN_ROOT'), 'PluginStatic')
    plugin_static = versioned_plugin_static_folder or base_plugin_static_folder
    os.environ['PLUGIN_STATIC'] = plugin_static

    base_license_folder = os.path.join(os.getenv('PINN_ROOT'), 'PinnacleSiteData', 'Licensing')
    versioned_license_folder = get_highest_version_folder(base_license_folder, 'Pinnacle')
    plugin_license = versioned_license_folder or base_license_folder
    os.environ['PLUGIN_LICENSE_DIR'] = plugin_license

    os.environ['GLKDIR'] = os.getenv('PINN_ROOT') + '/PinnacleSiteData/Licensing/'
    os.environ['PINN_HELP'] = os.getenv('PINN_ROOT') + '/Help'

    # Set LD_LIBRARY_PATH_64
    os.environ['LD_LIBRARY_PATH_64'] = os.getenv('PINN_STATIC') + '/LIBC/libc.so.1.SUN_122.3:' + \
                                       os.getenv('PINN_STATIC') + '/lib/' + arch + ':' + \
                                       os.getenv('PINN_STATIC') + '/bin/' + arch + ':' + \
                                       os.getenv('PINN_ROOT') + '/PluginStatic/lib/i386:' + \
                                       '/usr/dt/lib/amd64:/usr/ucblib/amd64:' + \
                                       '/usr/postgres/8.3-community/lib/64:' + \
                                       os.getenv('PINN_ROOT') + '/Lego/LegoStatic/lib/' + arch + ':' + \
                                       '/opt/Python345/lib:/usr/openwin/lib/64:/usr/dt/lib/64'
    # Set LD_LIBRARY_PATH
    os.environ['LD_LIBRARY_PATH'] = os.getenv('PINN_STATIC') + '/LIBC/libc.so.1.SUN_122.3:' + \
                                    os.getenv('PINN_STATIC') + '/lib/' + arch + ':' + \
                                    os.getenv('PINN_STATIC') + '/bin/' + arch + ':' + \
                                    os.getenv('PINN_ROOT') + '/PluginStatic/lib/i386:' + \
                                    '/usr/dt/lib:/usr/postgres/8.3-community/lib:/usr/openwin/lib'
    os.environ['PINN_LANG'] = 'English'
    os.environ['LC_CTYPE'] = 'en_US.ISO8859-1'
    os.environ['LC_COLLATE'] = 'en_US.ISO8859-1'
    os.environ['LC_MONETARY'] = 'en_US.ISO8859-1'
    os.environ['XNLSPATH'] = os.getenv('PINN_ROOT') + '/PinnacleStatic/bin/' + arch
    # os.environ['MERGE_INI'] = os.getenv('PINN_ROOT') + '/PinnacleStatic/DICOM/merge.ini'
    os.environ['MERGE_INI'] = home_dir + '/DICOMConfig/merge.ini'
    os.environ['XKEYSYMDB'] = '/usr/openwin/lib/XKeysymDB'
    os.environ['CURRENTDIR'] = os.getenv('PINN_ROOT') + '/LPStatic/bin'
    os.environ['PINN_STYLE'] = 'Plastique'
    os.environ['PINN_STYLESHEET'] = ':/qrc/experience_identity/Ei2010Dark.qss'
    os.environ['LCNUMERIC'] = 'C'


################################################################################
def moveFiles(fromDir, toDir, extension):
    ismoved = False
    for file in os.listdir(fromDir):
        if extension == '*' or file.endswith(extension):
            print("moving: ", file)
            shutil.move(fromDir + file, toDir)
            ismoved = True

    return ismoved


################################################################################
def writeScriptV16(defPinnScript, trialRx):
    with open(defPinnScript, "w") as f:
        f.write('PluginManager .ScriptingPlugin .Enable = "EnableScriptingVersion1.0";\n')
        f.write('PluginManager .ScriptingPlugin .AutoHandleMessages .BoolValue = 1;\n')

        f.write('SetCurrentTrial = "%s";\n' % trialRx[0])

        f.write('WindowList .DICOMExport .Create = "DICOM...";\n')
        f.write('PluginManager .DICOMExportPlugin .SendPlan = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SSDInArcs = 0;\n')

        f.write('PluginManager .DICOMExportPlugin .PrescriptionList .#"#%s" .SelectedForDICOMExport = 1;\n' % trialRx[1])
        f.write('TrialList .Current .UseTrialForTreatment = 1;\n')

        f.write('PluginManager .DICOMExportPlugin .SendStructures = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .RTStructureListSelected .#"#0" .Value = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .UpdateStructureList = 1;\n')

        f.write('UnSelectAllRois = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .RTStructureListSelected .#"#0" .Value = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .UpdateStructureList = 1;\n')
        f.write('SelectAllRoisOfDataSet = 1;\n')

        f.write('PluginManager .DICOMExportPlugin .SendDRR = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .AnnotateDRR = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendRTDose = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendRTDosePerPrescription = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendSpatialRegistration = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendDICOMImage = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .DICOMImageListSelected .#"#0" .Value = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .UpdateDICOMImageList = 1;\n')

        f.write('ExportDICOMRT = "";\n')
        f.write('QtWindowList .Common_TreeError .Unrealize = "";\n')
        f.write('Store.StringAt.ScriptName = "/home/kays/tarExtractor/pause.py";\n')
        f.write('PluginManager.ScriptingPlugin.DoPythonFile = Store.StringAt.ScriptName;\n')
        f.write('QtWindowList .DICOMExport .Unrealize = "";\n')
        #f.write('Quit = "";\n')


################################################################################
def writeScriptV9(defPinnScript, trialRx):
    with open(defPinnScript, "w") as f:
        f.write('Test .ExpectAskYesNo = 1;\n')
        f.write('Test .ExpectedAskYesNoReply = 1;\n')
        f.write('Test .ExpectedWarning = "1";\n')

        f.write('TrialList .Current = "%s";\n' % trialRx[0])
        f.write('WindowList .DICOMExport .Create = "DICOM...";\n')
        f.write('PluginManager .DICOMExportPlugin .SendPlan = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SSDInArcs = 0;\n')

        # f.write('TrialList .Current .PrescriptionList .#"#%s" .SelectedForDICOMExport = 1;\n' % trialRx[1])
        f.write('TrialList .Current .PrescriptionList .#"#0" .SelectedForDICOMExport = 1;\n')
        f.write('TrialList .Current .PrescriptionList .#"#1" .SelectedForDICOMExport = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendStructures = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .RTStructureListSelected .#"#0" .Value = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .UpdateStructureList = 1;\n')

        # f.write('PluginManager .DICOMExportPlugin .SendDRR = 1;\n')
        f.write('PluginManager .DICOMExportPlugin .SendRTDose = 1;\n')

        f.write('PluginManager .DICOMExportPlugin .SendRTDoseForPlan = 1;\n')
        # f.write('PluginManager .DICOMExportPlugin .SendRTDosePerPrescription = 1;\n')
        # f.write('PluginManager .DICOMExportPlugin .SendSpatialRegistration = 1;\n')
        # f.write('PluginManager .DICOMExportPlugin .SendDICOMImage = 1;\n')
        # f.write('PluginManager .DICOMExportPlugin .DICOMImageListSelected .#"#0" .Value = 1;\n')
        # f.write('PluginManager .DICOMExportPlugin .UpdateDICOMImageList = 1;\n')

        f.write('WindowList .DICOMExport .WidgetList .RTStructureList .Activate = "Transmit Data";\n')
        f.write('WindowList .DICOMExport .WidgetList .RTPrescriptionList .Activate = "Transmit Data";\n')
        # f.write('WindowList .DICOMExport .WidgetList .DICOMImageList .Activate = "Transmit Data";\n')
        f.write('WindowList .DICOMExport .WidgetList .SetupBeamList .Activate = "Transmit Data";\n')
        f.write('PluginManager .DICOMExportPlugin .RemoteAETitle = "ADACRTP_SCP_pinnconv";\n')
        f.write('DyeKom .AETitle = "ADACRTP_SCP_pinnconv";\n')
        f.write('WindowList .DICOMExport .WidgetList .RTAETitleOption .Activate = "Transmit Data";\n')
        f.write('ExportDICOMRT = "Transmit Data";\n')


################################################################################
def getStringIndex(markerTrial, planTrials, startOrEnd):
   index = []
   for match in re.finditer(markerTrial, planTrials):
        if (startOrEnd.lower() == 'start'):
            index.append(match.start())
        elif (startOrEnd.lower() == 'end'):
            index.append(match.end())
   
   return index


################################################################################
def getTrialRxNames(path, tarFileName):
    filename = path + '/plan.Trial'
    trialRxList = []

    if os.path.isfile(filename):
        f = open(filename)
        file = f.read()
        f.close()

        markerTrial = 'Trial ={\n  Name = '
        startTrialIndex = getStringIndex(markerTrial, file, 'end')

        markerTrial = '\n  PatientRepresentation ={'
        endTrialIndex = getStringIndex(markerTrial, file, 'start')

        markerTrial = 'PhysicsPlan = '
        startPhysicsPlanIndex = getStringIndex(markerTrial, file, 'end')

        markerTrial = ';\n  ComputeRelativeDose = '
        endPhysicsPlanIndex = getStringIndex(markerTrial, file, 'start')

        markerTrial = 'Prescription ={\n      Name = '
        startRxIndex = getStringIndex(markerTrial, file, 'end')

        markerTrial = '\n      RequestedMonitorUnitsPerFraction'
        endRxIndex = getStringIndex(markerTrial, file, 'start')

        trialNames = []
        if len(startTrialIndex) == len(endTrialIndex):
            for start, end in zip(startTrialIndex, endTrialIndex):
                tup = (start, 'trial', file[start+1: end-2])
                trialNames.append(tup)
        # print(trialNames)

        physicsPlans = []
        if len(startPhysicsPlanIndex) == len(endPhysicsPlanIndex):
            for start, end in zip(startPhysicsPlanIndex, endPhysicsPlanIndex):
                tup = (start, 'physicsplan', file[start+1: end-2])
                physicsPlans.append(tup)
        # print(physicsPlans)

        rxNames = []
        if len(startRxIndex) == len(endRxIndex):
            for start, end in zip(startRxIndex, endRxIndex):
                tup = (start, 'prescribe', file[start+1: end-2])
                rxNames.append(tup)

        # print(rxNames)

        trialRx = trialNames + physicsPlans + rxNames
        trialRx.sort()
        # print(trialRx)

        index = 0
        while index < len(trialRx):
            if trialRx[index][1] == 'trial':
                trial = trialRx[index][2]
                index += 1
                rxNum = -1
            elif trialRx[index][1] == 'prescribe':
                rx = trialRx[index][2]
                rxNum += 1
                index += 1
            elif trialRx[index][1] == 'physicsplan':
                physicsp = trialRx[index][2]
                rxNum += 1
                index += 1

            if rxNum > 0:
                tup = (trial, physicsp, rx)
                trialRxList.append(tup)

        print("trialRxList: ", trialRxList)

    else:
        if not os.path.isfile(tarFileName):
            with open(tarFileName, "w") as f:
                f.write('Referring to the tar file from which the RTDICOM are exported from\n')
                f.write('Trial exported: \n')

        if os.path.isfile(tarFileName):
            with open(tarFileName, "a") as f:
                f.write('============================== \n')
                f.write('Failed to export due to missing or incompatible plan.Trial \n')
                f.write('%s, %s\n' % (path, "false"))
                f.write('============================== \n')

    return trialRxList


################################################################################
def getCommandArgs(path, tarFileName):
    args = None
    file = path + '/plan.Command'
    if os.path.isfile(file):
        f = open(file)
        planCommand = f.read()
        f.close()

        index = planCommand.find('-plan')
        args = planCommand[index:]
    else:
        if not os.path.isfile(tarFileName):
            with open(tarFileName, "w") as f:
                f.write('Referring to the tar file from which the RTDICOM are exported from\n')
                f.write('Trial exported: \n')

        if os.path.isfile(tarFileName):
            with open(tarFileName, "a") as f:
                f.write('============================== \n')
                f.write('Failed to export due to missing or incompatible plan.Command \n')
                f.write('%s, %s\n' % (path, "false"))
                f.write('============================== \n')

    return args


################################################################################
def writeToLog(log_string, path, tarFileName):
    if not os.path.isfile(tarFileName):
        with open(tarFileName, "w") as f:
            f.write('Referring to the tar file from which the RTDICOM are exported from\n')
            f.write('Trial exported: \n')

    if os.path.isfile(tarFileName):
        with open(tarFileName, "a") as f:
            f.write('============================== \n')
            f.write('%s\n' %(log_string))
            f.write('%s, %s\n' % (path, "false"))
            f.write('============================== \n')


################################################################################
def createAndMoveRTDICOMs(RTDICOM_dir, tarFileName, planPath, MRN_dir):
    ismoved = False
    if not os.path.exists(RTDICOM_dir):
        os.makedirs(RTDICOM_dir)
    ismoved = moveFiles(os.environ['PINNLP_DATASETS'] + '/DICOM/', RTDICOM_dir, '.dcm')
  
    if not os.path.isfile(tarFileName):
        with open(tarFileName, "w") as f:
            f.write('Referring to the tar file from which the RTDICOM are exported from\n')
            f.write('Trial exported: \n')

    if os.path.isfile(tarFileName):
        with open(tarFileName, "a") as f:
            f.write('%s, %s\n' %(RTDICOM_dir, ismoved))

    shutil.copy(planPath + '/../Patient', RTDICOM_dir)

    plan_files = ['plan.Bolus',
                'plan.bolus.roi',
                'plan.Command',
                'plan.defaults',
                'plan.DoseVolHist',
                'plan.edit.roi',
                'plan.Isodose',
                'plan.Laser',
                'plan.OrbitBioConstraints',
                'plan.OrbitBioObjectives',
                'plan.OrbitConstraints',
                'plan.OrbitObjectives',
                'plan.PatientSetup',
                'plan.Pinnacle',
                'plan.Pinnacle.Machines',
                'plan.planInfo',
                'plan.planRev',
                'plan.Plugin.AutoReplan',
                'plan.Plugin.DeformationPlugin',
                'plan.Plugin.InverseplanningManager',
                'plan.Plugin.planEvalPlugin',
                'plan.Plugin.TTPlugin',
                'plan.Points',
                'plan.ReadOnlyRecord',
                'plan.roi',
                'plan.RoiManager',
                'plan.Stereo',
                'plan.Trial',
                'plan.VolumeInfo']

    for f in plan_files:
        file = planPath + '/' + f
        if os.path.isfile(file):
            shutil.copy(file, RTDICOM_dir)

    items = os.listdir(MRN_dir)
    for item in items:
        if item.startswith('Image') and item.endswith('.DICOM'):
            source_dir = MRN_dir + '/' + item
            link_dir = RTDICOM_dir + '/' + item
            if os.path.islink(link_dir):
                os.remove(link_dir)
            os.symlink(source_dir, link_dir)


################################################################################
def getPaths(tempDir, path):
    archData = ''.join(tempDir) + '/'
    # att = tempDir.rsplit('/', 1)[-1]
    # print('att: ', att)

    dirPath = ''.join(path)
    dirPath = dirPath.replace(archData, '')

    patient = dirPath.rsplit('/', -1)[-2]
    plan = dirPath.rsplit('/', -1)[-1]

    return dirPath, patient, plan


################################################################################
def getArchivePath(archive):
    year = archive.rsplit('/')[-3]
    month = archive.rsplit('/')[-2]
    archiveName = archive.rsplit('/')[-1]

    return year, month, archiveName

################################################################################
def checkIfConverted(path):
    isDicomed = False
    if os.path.exists(path):
       isDicomed = True

    return isDicomed
    
################################################################################
def getMRN(path):
    # print("getMRN: ", path)
    f = open(path + '/../Patient')
    file = f.read()
    f.close()

    marker = 'MRN = "'
    startIndex = getStringIndex(marker, file, 'end')
    # print('startIndex', startIndex[0])

    # marker = '";\n   };'
    # endIndex = getStringIndex(marker, file, 'start')
    endIndex = [indx + 8 for indx in startIndex]

    # endIndex.append(startIndex[0] + 8)
    # print('endIndex', endIndex[0])
    # print('patient index: ', startIndex, " ", endIndex)

    mrn = None 
    while len(startIndex) > 0 and len(endIndex) > 0:
        mrn = file[startIndex[0]: endIndex[0]]
        # print('while: ', mrn, " ",startIndex[0], " ",endIndex[0])
        if '"' in mrn:
            mrn = mrn.replace('"', '')
            break
        else:
            startIndex.pop(0)
            endIndex.pop(0)

    mrn = replace_special_char(mrn)

    marker = 'PlanID = "'
    startPlanIDIndex = getStringIndex(marker, file, 'end')
    marker = 'ToolType = '
    endPlanIDIndex = getStringIndex(marker, file, 'start')
    planIDs = []
    if len(startPlanIDIndex) == len(endPlanIDIndex):
        for start, end in zip(startPlanIDIndex, endPlanIDIndex):
            tup = (start, 'planID', file[start + 1: end - 2])
            planIDs.append(tup)
    # print(planIDs)

    marker = 'PlanIsLocked = "'
    startPlanIsLockedIndex = getStringIndex(marker, file, 'end')
    marker = 'OKForSyntegraInLaunchpad = '
    endPlanIsLockedIndex = getStringIndex(marker, file, 'start')
    planIsLockeds = []
    if len(startPlanIsLockedIndex) == len(endPlanIsLockedIndex):
        for start, end in zip(startPlanIsLockedIndex, endPlanIsLockedIndex):
            tup = (start, 'planIsLocked', file[start + 1: end - 2])
            planIsLockeds.append(tup)
    # print(planIsLockeds)

    lockedPlans = planIDs + planIsLockeds
    lockedPlans.sort()

    index = 0
    lockedPlansList = []
    while index < len(lockedPlans):
        if lockedPlans[index][1] == 'planID':
            planID = lockedPlans[index][2]
            index += 1
            rxNum = -1
        elif lockedPlans[index][1] == 'planIsLocked':
            locked = lockedPlans[index][2]
            rxNum += 1
            index += 1

        if rxNum > -1:
            tup = (planID, rxNum, locked)
            lockedPlansList.append(tup)

    print("lockedPlansList: ", lockedPlansList)

    marker = 'WriteTimeStamp = "'
    startIndex = getStringIndex(marker, file, 'end')
    endIndex = [indx + 19 for indx in startIndex]
    datestamp = file[startIndex[-1]: endIndex[-1]]
    datestamp = datestamp.replace(' ', '_')
    datestamp = datestamp.replace('-', '')
    datestamp = datestamp.replace(':', '')

    mrn = mrn + '-' + datestamp
    print('mrn: ', mrn + '-' + datestamp)
 
    return mrn, lockedPlansList


################################################################################
def copyDICOMImages(path, RTDICOM_dir):
    path = os.path.split(path)[0] + '/ImageSet_*.DICOM'
    print('copying ... ', path, ' ', RTDICOM_dir)

    if not os.path.exists(RTDICOM_dir):
        os.makedirs(RTDICOM_dir)

    cmd = "cp -r " + path + " " + RTDICOM_dir
    print(cmd)
    os.system(cmd)
    # shutil.copytree(path, RTDICOM_dir)


################################################################################
def pid_process(user, process):
    output = []
    cmd = "ps -aef | grep -i '%s' | grep -i '%s' | grep -v 'ADACRTP_SCP' | grep -v 'grep' | awk '{ print $2 }' > /tmp/out"
    os.system(cmd % (user, process))
    with open('/tmp/out', 'r') as f:
        line = f.readline()
        #print(line)
        while line:
            output.append(line.strip())
            line = f.readline()
            if line.strip():
                output.append(line.strip())

    return output


################################################################################
def replace_special_char(in_string):
    replace_list = ['~', '`', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '+', '=',
                    '<', '>', '?', ',', '/', ';', ':', '"', ' ']
    for char in replace_list:
        if char in in_string:
            in_string = in_string.replace(char, '_')

    return in_string


def keyboardInterruptHandler(signal, frame):
    print("KeyboardInterrupt (ID: {}) has been caught. Cleaning up...".format(signal))
    exit(0)


def termination(monitor, MRN_dir, userhomedir, archiveAll, tempDir):
    try:
        monitor.terminate()
    except OSError:
        pass
    monitor.wait()
    shutil.rmtree(MRN_dir)
    os.chdir(userhomedir)
    if archiveAll:
        shutil.rmtree(tempDir)


##############################################################################
# Title: main
# Executive loop to process archive file.
##############################################################################
def main():
    # signal.signal(signal.SIGINT, keyboardInterruptHandler)

    if len(sys.argv) != 3:
        print("*\n#\n*\n#\n*\n#")
        print("Usage: ", sys.argv[0], "<Archive Input>" "<DICOM output>")
        print("#\n*\n#\n*\n#\n*")
        exit(1)

    set_vars()
    userhomedir = os.getenv('HOME')
    firstTar = True

    for root, dirs, files in os.walk(sys.argv[1]):
        try:
            for file in files:
                if '.tar' in file:
                    archive = os.path.join(root, file)
                    year, month, archiveName = getArchivePath(archive)
                    print(year, " ", month, " ", archiveName)
                    print('archive: ', archive)

                    tarFileName = sys.argv[2] + '/' + year + '/' + month + '/' + archiveName
                    tarFileName = os.path.splitext(tarFileName)[0] + '.log'
                    """
                    if os.path.isfile(tarFileName):
                      print("already converted to DICOM")
                      continue
                    """

                    if not firstTar:
                        break

                    archiveAll = True
                    if archiveAll:
                         at = ArchiveTool(archive)
                         tempDir = at.tempDirectory
                         # use for debugging
                         # firstTar = False
                    else:
                         # use for debugging
                         # tempDir = '/home/kays/tarExtractor/archiveData'
                         tempDir = '/home/p3rtp/ExtractedArchiveData'

                    scan = DBScanner(tempDir)

                    # defShellScript = os.getenv('PINN_ROOT') + '/PinnacleStatic/bin/i386/StartPinnExec'
                    defShellScript = os.getenv('PINN_ROOT') + '/bin/StartPinnExec'
                    defPinnScript = 'RTDICOMexport.Script'

                    for i in scan.institutionList:
                        print('plan list: ', i.basePath)
                        os.environ['PINN_INSTITUTION'] = i.basePath.rsplit('/', -1)[-1]

                        if archiveAll:
                            subprocess.call(['chmod', '-R', '0777', at.tempDirectory])

                        if not os.path.islink(i.basePath + '/Physics'):
                            os.symlink('/home/p3rtp/Physics', i.basePath + '/Physics')

                        for p in scan.planList():
                            mrn, lockedPlansList = getMRN(p.basePath)
                            if mrn is None:
                                print("Something went wrong with reading of Patients, exiting")
                                break

                            print('plan: ', p.basePath)
                            print('MRN: ', mrn)
                            args = getCommandArgs(p.basePath, tarFileName)
                            if args is None:
                                print("plan.Command do not exist or failed to read, skipping this case")
                                continue

                            print('args: ', args)

                            dirPath, patient, plan = getPaths(tempDir, p.basePath)
                            plan = replace_special_char(plan)
                            print('dirPath: ', dirPath)
                            print('patient: ', patient)
                            print('plan: ', plan)

                            MRN_dir = sys.argv[2] + '/' + year + '/' + month + '/' + mrn

                            if os.path.exists(MRN_dir + '/' + plan):
                                print("already converted to DICOM")
                                continue

                            for planLocked in lockedPlansList:
                                if planLocked[0] == plan.replace('Plan_') and planLocked[1] != '1':
                                    writeToLog("plan is not locked", p.basePath, tarFileName)
                                    print("plan is not locked, skipping this")
                                    continue

                            trialRxNames = getTrialRxNames(p.basePath, tarFileName)
                            if len(trialRxNames) < 0:
                                print("Something went wrong with reading of plan.Trials, skipping this case")
                                continue
                            print('number of trials: ', len(trialRxNames))

                            try:
                                for trialRx in trialRxNames:
                                    if trialRx[1] == "0":
                                        writeToLog("QA or Evaluation trial", p.basePath, tarFileName)
                                        print("QA or Evaluation trial, skipping this")
                                        continue

                                    RTDICOM_dir = sys.argv[2] + '/' + year + '/' + month + '/' + mrn + '/' + plan + '/' \
                                                  + replace_special_char(trialRx[0]) + '/' \
                                                  + replace_special_char(trialRx[2])
                                    if not os.path.exists(RTDICOM_dir):
                                        os.makedirs(RTDICOM_dir)

                                    pinnacle_log = RTDICOM_dir + '/pinnacle_run.log'
                                    writeScriptV9(defPinnScript, trialRx)
                                    commandLine = [defShellScript,
                                                   os.environ['PINNACLE_EXEC'],
                                                   '-research -dir ' + dirPath,
                                                   ' ' + args,
                                                   '-script ../' + defPinnScript,
                                                   '3>&2 2>&1 1>&3',
                                                   '| tee ' + pinnacle_log]

                                    os.chdir(tempDir)
                                    monitorcmd = ['python', '../MonitorArchive2Dicom.py']
                                    monitor = subprocess.Popen(monitorcmd)

                                    print(os.getcwd())
                                    print("commandLine: ", ' '.join(commandLine))
                                    os.system(' '.join(commandLine))

                                    copyDICOMImages(p.basePath, MRN_dir)
                                    createAndMoveRTDICOMs(RTDICOM_dir, tarFileName, p.basePath, MRN_dir)

                                    os.chdir('../')

                            except KeyboardInterrupt:
                                print("terminated ... removing ", MRN_dir)
                                termination(monitor, MRN_dir, userhomedir, archiveAll, tempDir)
                                exit(0)

                    if archiveAll:
                        at.clearUp()

        except KeyboardInterrupt:
            print("terminated ... removing ", MRN_dir)
            termination(monitor, MRN_dir, userhomedir, archiveAll, tempDir)
            exit(0)

if __name__ == '__main__':
    main()



