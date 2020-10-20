import json
import urllib.request
import requests
import math
import sys

org_name = "OPM"
repo_name = "ResInsight"
keep_artifacts = 20

def get_all_artifacts(repo_name: str, headers: dict) -> []:
    amount_items_per_page = 50
    page = 1

    ref = "https://api.github.com/repos/" + org_name + "/" + repo_name + "/actions/artifacts" + "?per_page=" + str(amount_items_per_page)

    result = requests.get(ref, headers=headers).json()
    #print(json.dumps(result, indent=4, sort_keys=True))
        
    done = False

    all_artifacts = []

    while not done:
        total_count = int(result["total_count"])

        if total_count == 0:
            done = True

        all_artifacts = all_artifacts + result["artifacts"]

        max_pages = math.ceil(total_count / amount_items_per_page)

        if max_pages > page:
            page = page + 1
        else:
            done = True

    return all_artifacts


def delete_artifact(repo_name: str, artifact_id: str, headers: dict) -> bool:
    try:
        res = urllib.request.urlopen(urllib.request.Request(
            url="https://api.github.com/repos/" + org_name + "/" + repo_name + "/actions/artifacts/" + artifact_id,
            headers=headers,
            method='DELETE')
        ).getcode()
        return res == 204
    except urllib.error.URLError as error:
        print(error)
        return False


length = len(sys.argv)

if length < 2:
    print('This script requires Github access token as argment')
    sys.exit()

access_token = sys.argv[1]
headers = {"content-type": "application/json; charset=UTF-8",'Authorization':'Bearer {}'.format(access_token)}

artifacts = get_all_artifacts(repo_name, headers)
print("Amount of artifacts for repo " + repo_name + " is " + str(len(artifacts)))

if len(artifacts) > keep_artifacts:
    for a in artifacts[keep_artifacts:]:
        artifact_id = str(a["id"])
        deleted = delete_artifact(repo_name, artifact_id, headers)

        if deleted:
            print("Repo: " + repo_name + " | Artifact with id " + artifact_id + " has been deleted")
        else:
            print("Repo: " + repo_name + " | Something went wrong while deleting artifact with id " + artifact_id)
            
