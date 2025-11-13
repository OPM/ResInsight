# MSW test and verification

This folder contains test data taken from 

    https://github.com/OPM/opm-tests/tree/master/model1

Project files with different completion configuration is defined, and can be used to verify the export of MSW data.

1. In Preferences->System->Developer Settings, in "Keywords to enable experimental feagures separated by semicolon.", enter the text "enable-all" 
2. Open a project from project-files folder
3. Select `Well-1`, and export completion. Make sure to select "Split on well"
4. Exported text files are witten to the `output` folder. Experimental MSW is written to `output/prototype` folder
5. Compare the text output from these two folders

