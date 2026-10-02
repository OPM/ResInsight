# Setting up credentials for the dev-builds distribution job

The `publish-dev-build` job in [`ResInsightWithCache.yml`](../.github/workflows/ResInsightWithCache.yml)
publishes `dev` build artifacts as assets on a rolling pre-release in a
separate distribution repository ([OPM/ResInsight-Builds](https://github.com/OPM/ResInsight-Builds)),
giving users a permanent, unauthenticated download URL served by GitHub's
CDN instead of relying on nightly.link and expiring Actions artifacts (see
magnesj/ResInsight#1093).

This only works if the repository running the workflow has a token that is
allowed to create releases and upload assets in the distribution repository.
Anyone forking this setup for their own repository needs to configure that
token once, as described below.

## 1. Create the distribution repository

Create a repository to hold the published builds, e.g.
`<your-account>/<project>-dev-builds`. It only needs a `README.md`; the CI
job creates and updates the `dev-latest` release automatically.

## 2. Create a fine-grained personal access token (PAT)

1. Go to **GitHub → Settings → Developer settings → Personal access tokens →
   Fine-grained tokens** and create a new token.
2. **Resource owner**: the account/organization that owns the distribution
   repository.
3. **Repository access**: "Only select repositories" → choose the
   distribution repository only (do *not* grant access to the main source
   repository with this token).
4. **Permissions**: under "Repository permissions", set **Contents** to
   **Read and write**. This is the only permission needed to create/edit
   releases and upload/replace assets.
5. Set an expiration date and copy the generated token value. A GitHub App
   installation token with the same `contents: write` scope works as well and
   avoids manual renewal.

## 3. Store the token as a secret in the source repository

In the repository that runs the build workflow (the one containing
`ResInsightWithCache.yml`):

1. Go to **Settings → Secrets and variables → Actions → New repository
   secret**.
2. Name: `DEV_BUILDS_TOKEN`.
3. Value: the PAT (or App token) created in step 2.

## 4. Point the workflow at your distribution repository

The job reads the target repository from the `GH_REPO` environment variable
in the "Update rolling release in builds repo" step. Update it to match the
distribution repository created in step 1, e.g.:

```yaml
env:
  GH_TOKEN: ${{ secrets.DEV_BUILDS_TOKEN }}
  GH_REPO: <your-account>/<project>-dev-builds
  TAG: dev-latest
```

## 5. Verify

Push to the branch the job is scoped to (normally `dev`). The job should
create (first run) or update (subsequent runs) the `dev-latest` pre-release
in the distribution repository, with assets renamed to stable, CI-agnostic
names (the release description documents which CI build configuration each
asset was packaged from and links back to the source build job), e.g.:

```
https://github.com/<your-account>/<project>-dev-builds/releases/download/dev-latest/ResInsight-Ubuntu.zip
```

## Notes

- The token should be scoped **only** to the distribution repository, never
  to the main source repository, to limit the blast radius if it leaks.
- `gh release upload --clobber` deletes the old asset before uploading the
  new one, so a download during that window of a few seconds gets a 404.
  This is generally acceptable for rolling dev builds.
- Rotate the PAT before it expires, or use a non-expiring GitHub App
  installation token instead.
