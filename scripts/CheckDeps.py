#!/usr/bin/env python3
import csv
import json
import os
import re
import sys

includeRegex = re.compile(r"^\s*#\s*include\s+[\"<]([^ ]*)[\">]\s*$")
ErrorCount = 0

def main(argv):
    global ErrorCount
    global NodeDependencies

    def popArg():
        nonlocal argv
        if len(argv) == 0:
            raise Exception("Unexpected end of command line")
        result = argv[0]
        argv = argv[1:]
        return result

    def accept(optionName):
        nonlocal argv
        if len(argv) == 0 or argv[0] != optionName:
            return False
        popArg()
        return True

    progName = popArg()

    if len(argv) < 2:
        print(f"Usage: {progName} [-o <output.dot>] <archi.json> [dirs...]")
        return 1

    dotPath = ""
    if accept("-o"):
        dotPath = popArg()

    configPath = popArg()
    parseConfig(configPath)

    if dotPath != "":
        dumpAsDot(NodeDependencies, dotPath)

    if not checkForCyclesInConfig():
        return 1

    for path in getAllSourceFiles(argv):
        checkOneSourceFile(path)

    return ErrorCount > 0

def checkOneSourceFile(path):
    global ProjectList
    global NodeDependencies

    myProjectName = extractProjectName(path)

    if not myProjectName in ProjectList:
        doError(f"'{path}': unknown project '{myProjectName}'")
        return

    myNodeName = myProjectName
    myDeps = NodeDependencies[myNodeName]

    for inclusion in getInclusions(path):
        elements = inclusion.split("/")

        # ignore std includes
        if len(elements) <= 1:
            continue

        otherProjectName = elements[0]
        if not otherProjectName in ProjectList:
            doError(f"'{path}': depends on unknown project '{otherProjectName}' through '{inclusion}'")
            continue

        if not otherProjectName in myDeps:
            doError(f"'{path}': from project '{myProjectName}' must not depend on '{otherProjectName}', but includes '{inclusion}'")
            continue

def doError(msg):
    global ErrorCount
    ErrorCount += 1
    print(f"Error: {msg}")

def parseConfig(path):
    global ProjectList
    global NodeDependencies
    ProjectList = []
    NodeDependencies = dict()

    jsonConfig = json.loads(open(path).read())
    for row in jsonConfig:
        nodeName = row["name"]
        NodeDependencies[nodeName] = []
        if "deps" in row:
            NodeDependencies[nodeName] = row["deps"]
        ProjectList.append(nodeName)

def getInclusions(path):
    global includeRegex
    result = []
    for line in open(path, 'r', encoding='utf-8', errors='ignore').readlines():
        m = includeRegex.match(line)
        if m:
            includedHeader = m.groups()[0]
            result.append(includedHeader)
    return result

def extractProjectName(path):
    relativePath = re.sub(r'^src/', '', path)
    if relativePath == path:
        return ""

    elements = relativePath.split("/")
    if len(elements) <= 1:
        return ""

    return elements[0]

def getAllSourceFiles(dirs):
    possibleFileExtensions = [ ".cpp", ".h" ]
    result = []

    for directory in dirs:
        sf, files = scanDirectory(directory, possibleFileExtensions)
        result.extend(files)

    return result

def scanDirectory(directory, extensions):
    subfolders, files = [], []

    for f in os.scandir(directory):
        if f.is_dir():
            subfolders.append(f.path)
        if f.is_file():
            if os.path.splitext(f.name)[1] in extensions:
                files.append(f.path)

    for directory in list(subfolders):
        sf, f = scanDirectory(directory, extensions)
        subfolders.extend(sf)
        files.extend(f)
    return subfolders, files

def checkForCyclesInConfig():
    cycle = findOneCycle(NodeDependencies)
    if cycle:
        doError("Found cycle in config file: " + " -> ".join(cycle))
        return False
    return True

def findOneCycle(graph):
    alreadyExplored = set()

    def browse(stack):
        node = stack[-1]
        alreadyExplored.add(node)

        if node not in graph:
            doError(f"Unknown project '{node}'")
            return []

        for child in graph[node]:
            if child in stack:
                return stack + [child] # cycle found
            if not child in alreadyExplored:
                cycle = browse(stack + [child])
                if cycle:
                    return cycle
        return None # no cycle found

    for node in graph:
        cycle = browse([node])
        if cycle:
            return cycle

    return None

def dumpAsDot(graph, filename):
    with open(filename, "w") as of:
        of.write("digraph {")
        for node in graph:
            deps = []
            for dep in graph[node]:
                deps.append(dep)
            if len(deps) > 0:
                of.write('"%s" -> "%s";\n' % (node, '", "'.join(deps)))
        of.write("}")
    pass

sys.exit(main(sys.argv))
