"""Electronics Supplier Data Processing — NC(V) Technical Programming PAT 2.

This is a cleaned-up reference implementation based on the corrected PAT2
solution in the study material. It expects the input text files described
below. It has not been tested against the lecturer's original data files.
"""

from pathlib import Path
import math

DATA_FILE = Path("components_data.txt")
ERROR_DATA_FILE = Path("components_data_ERR.txt")
ERROR_LOG_FILE = Path("error_log.txt")


def filter_components(data_file: Path = DATA_FILE) -> None:
    """Display component records whose IDs start with a chosen letter."""
    letter = input("Enter a letter (a-z): ").strip()

    if len(letter) != 1 or not letter.isalpha():
        print("Error: Please enter a single letter from a to z.")
        return

    letter = letter.lower()

    try:
        with data_file.open("r", encoding="utf-8") as file:
            print("\nFiltered Components:\n")
            found = False

            for line_number, line in enumerate(file, start=1):
                line = line.strip()
                if not line:
                    continue

                parts = [part.strip() for part in line.split(",")]
                if len(parts) != 2:
                    print(f"Skipped malformed record on line {line_number}.")
                    continue

                component_id, voltage_reading = parts
                if component_id.lower().startswith(letter):
                    print(
                        f"Component ID: {component_id}, "
                        f"Voltage: {voltage_reading}"
                    )
                    found = True

            if not found:
                print(f'No components found with IDs starting with "{letter.upper()}".')

    except FileNotFoundError:
        print(f"Error: {data_file.name} file not found.")


def validate_entry(component_id: str, voltage_reading: str) -> tuple[bool, str]:
    """Validate a four-character alphanumeric ID and a numeric voltage."""
    if len(component_id) != 4 or not component_id.isalnum():
        return False, "Invalid component ID; it must be 4 alphanumeric characters."

    if not voltage_reading:
        return False, "Missing voltage reading."

    try:
        voltage = float(voltage_reading)
    except ValueError:
        return False, "Non-numeric voltage reading."

    if not math.isfinite(voltage):
        return False, "Voltage reading must be a finite number."

    return True, ""


def process_file(
    data_file: Path = ERROR_DATA_FILE,
    error_log_file: Path = ERROR_LOG_FILE,
) -> None:
    """Check each record and write invalid entries to an error log."""
    invalid_entries = 0

    try:
        with data_file.open("r", encoding="utf-8") as source, error_log_file.open(
            "w", encoding="utf-8"
        ) as error_log:
            for line_number, line in enumerate(source, start=1):
                line = line.strip()
                if not line:
                    continue

                parts = [part.strip() for part in line.split(",")]
                if len(parts) != 2:
                    component_id = parts[0] if parts else ""
                    voltage_reading = ""
                    valid = False
                    message = "Invalid record format; expected ID,voltage."
                else:
                    component_id, voltage_reading = parts
                    valid, message = validate_entry(component_id, voltage_reading)

                if not valid:
                    invalid_entries += 1
                    error_log.write(f"Line {line_number}: {message}\n")

    except FileNotFoundError:
        print(f"Error: {data_file.name} file not found.")
        return

    if invalid_entries == 0:
        print(f"{data_file.name} is free of errors. Development can continue.")
    else:
        print(
            f"{invalid_entries} invalid entr{'y' if invalid_entries == 1 else 'ies'} "
            f"found. Please check {error_log_file.name}."
        )


if __name__ == "__main__":
    print("1. Filter components")
    print("2. Validate the error-data file")
    choice = input("Choose an option (1 or 2): ").strip()

    if choice == "1":
        filter_components()
    elif choice == "2":
        process_file()
    else:
        print("Invalid choice. Please enter 1 or 2.")
