__author__ = 'ja'

import xml.etree.cElementTree as ET
from optparse import OptionParser
import os

parser = OptionParser()
parser.add_option("-p", "--path", dest="path",
                  help="path to eos", metavar="FILE")

(options, args) = parser.parse_args()

root = ET.Element("root")

for dirname, dirnames, filenames in os.walk(options.path):
    if '.git' in dirnames:
        # don't go into any .git directories.
        dirnames.remove('.git')
    for subdirname in dirnames:
        if "RawData" in subdirname:
            boardName=dirname[dirname.rfind("/")+1:]
            directory = ET.SubElement(root, "directory", name=boardName)
            Pedestal=ET.SubElement(directory, "Pedestal",)
            Bias=ET.SubElement(directory, "BiasScan",)
            Angle=ET.SubElement(directory, "AngleScan",)

    for filename in filenames:
        if "Bias" in filename and ".xml" not in filename:
            ET.SubElement(Bias, "file", ).text = filename
        elif "Angle" in filename and ".xml" not in filename:
            ET.SubElement(Angle, "file", ).text = filename
        elif "Pedestal" in filename and ".xml" not in filename:
            ET.SubElement(Pedestal, "file", ).text = filename

tree = ET.ElementTree(root)
tree.write("July2015Database.xml")