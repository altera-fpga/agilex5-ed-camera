#!/usr/bin/env python3
# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************

import struct
import sys
from pathlib import Path

# To convert the output of this script to an object for linking:
# $LD -r -b binary -o WebServerHome.o WebServerHome.bin

class FileBlobHeader:
    def __bytes__(self):
        blob = bytearray()
        blob += struct.pack('<I', self._size)
        blob += struct.pack('<{}s'.format(FileBlobHeader._filenameSize), bytes(self._relativeFileName, 'utf-8'))
        return bytes(blob)

    def HeaderSize():
        return 4 + FileBlobHeader._filenameSize

    _filenameSize=128
    _size = 0
    _relativeFileName=""

if len(sys.argv) != 3:
    print("Usage: %s <source-folder> <output-file>\n".format(sys.argv[0]))
    sys.exit(-1)

fileBlobHeaderSize = FileBlobHeader.HeaderSize()

startFolderStr = sys.argv[1]
outputFilename = sys.argv[2]

pathlist = Path(startFolderStr).glob('**/*')

# Allocate a blob to contain all the files, with their headers
bigBlob = bytearray()

result = True
filenameSize = FileBlobHeader._filenameSize

supportedExtensions = ['.html', '.htm', '.css', '.js', '.png', '.ico', '.ttf']

for filePath in pathlist:
    if filePath.is_file():
        fileExtension = filePath.suffix
        if fileExtension.lower() not in supportedExtensions:
            continue

        pBlobHeader = FileBlobHeader()
        pBlobHeader._size = filePath.stat().st_size + FileBlobHeader.HeaderSize()

        # Compute relative path from start folder
        relativePath = filePath.relative_to(startFolderStr)

        # Copy the relative path name into the header
        pBlobHeader._relativeFileName = str(relativePath)
        print(pBlobHeader._relativeFileName)

        # Read each file into the blob
        try:
            with open(filePath, mode='rb') as file:
                fileContent = file.read()
                bigBlob += bytes(pBlobHeader)
                bigBlob += fileContent
        except:
            print("Failed to read file '{}'".format(filePath))
            result = False
            break

if result:
    try:
        with open(outputFilename, mode='wb') as file:
            file.write(bigBlob)
    except:
        print("Failed to write to file '{}'".format(outputFilename))
        result = False

if result:
    sys.exit(0)
else:
    sys.exit(-1)
