---
method: constructor
summary: "Open a file specified by path with the specified mode."
synopsis: "Irccd.File(path, mode) /* constructor */"
arguments:
  - "path, the path to the file,"
  - "mode, the mode, can be one of [abrwt]"
throws: "Any exception on error"
---

The mode string understands the following characters:

  - **a**: append to the end of file,
  - **b**: open in binary mode,
  - **r**: open for reading,
  - **w**: open for writing,
  - **t**: truncate the file.