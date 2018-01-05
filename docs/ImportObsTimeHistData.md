---
layout: docs
title: Import Observed Time History Data
permalink: /docs/importobstimehistdata/
published: true
---

Importing observed time history data to ResInsight may be performed in two different ways:
- By selecting the main menu item **File -> Import -> Import Observed Time History Data**
- By using the context command **Import Observed Time History Data** on the **Observed Time History Data** item in the **Plot Main Window Project Tree** 

The following file types are supported:
- RSM observed time history data file (_\*.rsm_)
- Column based (Comma Separated Values, CSV) time history data file (_\*.csv/\*.txt_)

## Import RSM observed time history data
To import RSM files, the only action needed from the user is to select one or more RSM files. When the import is finished, one tree node for each file will appear under the **Observed Time History Data** node in the project tree.

## Import CSV/txt observed time history data
CSV/txt files are generic ascii files which may have slightly different formatting. When importing these types of files the user is presented a dialog, where the user may tell ResInsight how to import the selected file(s). If more than one file is selected, the dialog appears once for each file.

### CSV/txt import options dialog
![]({{ site.baseurl }}/images/ImportObservedTimeHistoryDataDialog.png)

Dialog fields description:
- **Cell Separator** -- Select the correct cell separator character. ResInsight will try to set the correct value as default.
- **Decimal Separator** -- Select the correct decimal separator. ResInsight will try to set the correct value as default.
- **Selected Time Column** -- Select the column that contains the time/date information. The first column is default.
- **Use Custom Date Time Format** -- Check this box if the Date Format and/or Time Format in the file do not match any of the most common formats.
- **Custom Date Time Format** -- Enter date time format to match the time information in the file. This field is visible only when the above check box checked. A tooltip will tell the user how to enter the correct information.
- **Date Format** -- Select the date format matching the date information in the file.
- **Time Format** -- Select the time format matching the time information in the file. If the file contains dates only, this field is ignored by ResInsight.
- **Preview** -- Preview the first 30 lines of the file contents. The view will reflect the currently selected Cell Separator and the selected time column is marked in yellow.
