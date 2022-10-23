#/usr/bin/env python3

from io import TextIOWrapper
from operator import contains
from os import path, remove, makedirs
from glob import glob
from typing import List

boardsDir = '../boards/'
displaysDir = '../displays/'
envsDir = '../env/'
fileExt = '.ini'

def main():
	if (not path.exists(envsDir)):
		makedirs(envsDir)

	files = glob(envsDir + '*')
	for file in files:
			remove(file)

	for boardFilePath in glob(path.join(boardsDir, '*' + fileExt)):
		with open(boardFilePath) as boardFile:
			boardName = boardFile.name[len(boardsDir):(len(boardFile.name) - 4)]
			boardSection = parsePIOSection(boardFile)
			for displayFilePath in glob(path.join(displaysDir, '*' + fileExt)):
				with open(displayFilePath) as displayFile:
					displayName = displayFile.name[len(displaysDir):(len(displayFile.name) - 4)]
					displaySection = parsePIOSection(displayFile)

					envName = boardName + '-' + displayName
					boardOptions = list(set(boardSection.options) - set(displaySection.options))
					displayOptions = list(set(displaySection.options) - set(boardSection.options))
					commonOptions = set(boardSection.options).intersection(set(displaySection.options))

					with open(path.join(envsDir, envName + fileExt), 'x') as envFile:
						envFile.write('[env:' + envName + ']\n')
						for boardOption in boardOptions:
							envFile.write(boardOption)
							envFile.write(' = ')
							envFile.write('${' + boardSection.name + '.' + boardOption + '}\n')
						for displayOption in displayOptions:
							envFile.write(displayOption)
							envFile.write(' = ')
							envFile.write('${' + displaySection.name + '.' + displayOption + '}\n')
						for commonOption in commonOptions:
							envFile.write(commonOption)
							envFile.write(' = \n')
							envFile.write('\t${' + displaySection.name + '.' + commonOption + '}\n')
							envFile.write('\t${' + boardSection.name + '.' + commonOption + '}\n')
						envFile.write('upload_port = .pio/build/' + envName + '/\n')


def parsePIOSection(file: TextIOWrapper):
	class PIOSection:
		name = ''
		options = []

	# First line without a comment should be the section name: [SECTION]
	lines = file.readlines()
	result = PIOSection()
	for line in lines:
		# Skip lines that likely don't have a parameter
		if (line.startswith('\t') or line.startswith(' ')):
			continue

		line = line.strip()

		# Skip comments
		if (line.startswith(';')):
			continue

		# Check if it's the name
		if (line.startswith('[')):
			result.name = line.strip('[]').strip()
		# Treat it as a parameter if it looks like one
		elif (not line.startswith('-') and '=' in line):
			result.options.append(line[0:line.index('=')].strip())

	return result


if __name__ == "__main__":
	main()
