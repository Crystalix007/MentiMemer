# MentiMemer
Exercising your vote(s)

## To build
If you have `tup` installed (for building with `tupfiles`): <br />
`tup` <br />
Else: <br />
`./Build.sh`

## To run (in mitm mode):
`./Startup.sh <wireless hotspot here sharing internet connection from somewhere else>`

## To run manually:
`./MentiMemer` and follow the prompts. <br />
Here, "gameCode" represents the end of the URL, i.e. `f1758b15` from `https://www.menti.com/f1758b15`. <br />
Also, "questionId" "questionType" "answer" come from the actual request sent to Menti. <br />
This can be viewed in your browser of choice by opening developer tools / developer console, submitting the response, and viewing the _request_ data for the HTTP POST request to `https://www.menti.com/core/vote`.
