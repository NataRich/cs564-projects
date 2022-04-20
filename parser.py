import sys
from json import loads

from data_models import Parser


parser = Parser()


def isJson(f):
    """
    Returns true if a file ends in .json
    """

    return len(f) > 5 and f[-5:] == '.json'


def parseJson(json_file):
    """
    Parses a single json file and creates two log reports.
    """
    parser.set_file(json_file)
    with open(json_file, 'r') as f:
        items = loads(f.read())['Items'] # creates a Python dictionary of Items for the supplied json file
        parser.parse(items)
    parser.report()


def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: python parser.py <path to json files>'
        sys.exit(1)
    # loops over all .json files in the argument
    for f in argv[1:]:
        if isJson(f):
            parseJson(f)
            print("Success parsing " + f)

    parser.flush()
    print("Success all")


if __name__ == '__main__':
    main(sys.argv)
