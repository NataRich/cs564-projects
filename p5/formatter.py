import os
import json

WORKIN_DIR = os.getcwd()
TARGET_DIR = os.path.join(WORKIN_DIR, 'fmt')  # formatted json files go into ./fmt

if __name__ == '__main__':
    # check if the target directory exists
    # if does, user must remove it first
    if os.path.exists(TARGET_DIR):
        print('The target directory has been created:')
        print(f'    {TARGET_DIR}')
        print('Please remove the directory before continuing.')
        exit(0)

    # create the target directory
    os.makedirs(TARGET_DIR)
    print(f'The target directory was successfully created:')
    print(f'    {TARGET_DIR}')

    # format json files
    files = [f'items-{i}.json' for i in range(0, 40)]
    targets = [os.path.join(TARGET_DIR, f) for f in files]
    for file, target in zip(files, targets):
        with open(file, 'r') as oldf:
            raw = json.load(oldf)
            with open(target, 'w') as newf:
                json.dump(raw, newf, indent=4)
        print(f'Saved formatted {file} into {target}')
    print('Completed formatting.')

