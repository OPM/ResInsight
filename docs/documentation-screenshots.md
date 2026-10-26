# Screenshots for the User Documentation

When a feature is finished, add screenshots to the issue so they can be fetched into the
[ResInsight user documentation](https://github.com/OPM/ResInsight-UserDocumentation). The script
`scripts/fetch_issue_images.py` in that repository downloads them to `assets/images/` and records a caption
and target page for each image.

## Steps

1. Add the **Documentation** label to the issue. The script processes issues labelled both
   **Documentation** and **PendingRelease**.
2. Post a new comment on the issue that starts with `### Documentation`.
3. Optionally add a `Page:` line naming the documentation page to update.
4. Paste each screenshot, then edit its alt text to the target path (see below).
5. Write the caption on the line directly below each image.

## Example

```markdown
### Documentation

Page: plot-window/SummaryPlots.md

<img width="1278" alt="plot-window/SummaryPlotCommonLegend.png" src="https://github.com/user-attachments/assets/..." />
Common legend shown once for all subplots.

<img width="640" alt="plot-window/SummaryPlotColorAndSymbolLegend.png" src="https://github.com/user-attachments/assets/..." />
Separate color and symbol legends when color and symbol are given by different summary dimensions.
```

Markdown image syntax works as well: `![plot-window/SummaryPlotCommonLegend.png](https://github.com/user-attachments/assets/...)`.

## Rules

- **Heading**: The comment must start with `### Documentation`. Only the newest such comment on an issue is
  used, so edit that comment or post a complete new one to make changes. The issue description is never read.
- **Alt text is the target path** below `assets/images/`, in the form `<section>/<Name>.png` or
  `<section>/<Name>.jpg`:
  - `<section>` must be an existing folder: `3d-main-window`, `appendix`, `calculated-data`,
    `cloud-services`, `export`, `getting-started`, `import`, `introduction`, `misc`,
    `opm-flow-integration`, `plot-window`, `release-notes`, `scripting`, `surface`, `well-modeling`,
    `workflows`.
  - `<Name>` uses PascalCase, with letters, digits, `_`, `-` and `.` only. The extension must be lowercase.
  - GitHub sets `alt="Image"` when pasting. Replace it with the path.
  - Reusing an existing image name replaces that image, so pages linking to it stay valid.
- **Images with other alt text are ignored** (alt text without `/`). Screenshots used for discussion are not
  picked up. Alt text containing `/` that is not a valid path is reported as an error.
- **Caption**: The text lines directly below an image, up to the next blank line, form its caption. The
  blank line GitHub inserts right after a pasted image is allowed.
- **Page** (optional): `Page: <section>/<file>.md`, relative to `content/` in the documentation repository.
  It applies to all images in the comment. A page that does not exist is treated as a new page.
- Take screenshots from a current `dev` build, cropped to the relevant part of the UI.

## Check the comment

From a clone of the documentation repository, list what would be fetched without downloading:

```powershell
$env:GITHUB_TOKEN = gh auth token
python scripts/fetch_issue_images.py <issue-number> --dry-run
```
