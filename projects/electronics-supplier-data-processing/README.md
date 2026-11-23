# Electronics Supplier Data Processing — Technical Programming PAT 2

**Language:** Python  
**Qualification:** NC(V) Level 4 Technical Programming  
**Status:** Reference implementation prepared from the corrected solution document; original lecturer data files and execution tests are still required.

## Features
- Filters component records by the first letter of the component ID.
- Validates a four-character alphanumeric component ID.
- Checks that voltage readings are numeric and finite.
- Writes invalid records and line numbers to `error_log.txt`.
- Handles missing input files and malformed records.

## Files
- [main.py](main.py) — cleaned-up reference implementation.

## Required input files
Place the following files in the same folder as `main.py`:
- `components_data.txt` — records in `componentID,voltage` format.
- `components_data_ERR.txt` — records to validate.

These data files were not available with the source material used to prepare this repository. Do not treat this program as fully verified until it is tested against the official PAT2 data and marking requirements.

## Run
```bash
python main.py
```

Choose option 1 to filter records or option 2 to validate the error-data file.
