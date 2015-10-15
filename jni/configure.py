# -*- coding: utf-8 -*-
import re

version = {}
version['libmodbus_version_major'] = 0
version['libmodbus_version_minor'] = 0
version['libmodbus_version_micro'] = 0

pattern = re.compile('m4_define\(\[(libmodbus_version_m[a-z]+)\], \[([0-9]+)\]\)');

## Read the version numbers from ../configure.ac
with open('configure.ac', 'r') as input:
  for line in input:
    match = pattern.match(line)
    if match:
      version[match.group(1)] = match.group(2)

## Prepare the regex-patterns so we don't have to do that multiple times
version_pattern = re.compile('@LIBMODBUS_VERSION@')
version_major_pattern = re.compile('@LIBMODBUS_VERSION_MAJOR@')
version_minor_pattern = re.compile('@LIBMODBUS_VERSION_MINOR@')
version_micro_pattern = re.compile('@LIBMODBUS_VERSION_MICRO@')

## Read jni/config.h.in and write jni/config.h
infile =  open('jni/config.h.in', 'r')
content = infile.read()
infile.close()

outfile = open('jni/config.h', 'w')

content = version_pattern.sub(version['libmodbus_version_major'] + '.' + version['libmodbus_version_minor'] + '.' + version['libmodbus_version_micro'], content)

outfile.write(content)
outfile.close()

## Read src/modbus-version.h.in and write jni/modbus-version.h
infile =  open('src/modbus-version.h.in', 'r')
content = infile.read()
infile.close()

outfile = open('jni/modbus-version.h', 'w')

content = version_pattern.sub(version['libmodbus_version_major'] + '.' + version['libmodbus_version_minor'] + '.' + version['libmodbus_version_micro'], content)
content = version_major_pattern.sub(version['libmodbus_version_major'], content)
content = version_minor_pattern.sub(version['libmodbus_version_minor'], content)
content = version_micro_pattern.sub(version['libmodbus_version_micro'], content)

outfile.write(content)

outfile.close()
