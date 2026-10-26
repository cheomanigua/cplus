#!/usr/bin/env python3
"""
CSV to JSON Converter.

Converts CSV files from the csv/ directory into JSON files
in the json/ directory.
Uses only the Python standard library.
"""

import csv
import json
from pathlib import Path
from typing import Any


def csv_to_json_dict(csv_path: Path) -> dict[str, Any]:
    """Convert a CSV file into a nested JSON-compatible dictionary."""
    data: dict[str, Any] = {}

    with csv_path.open("r", encoding="utf-8", newline="") as file:
        reader = csv.DictReader(file)

        for row in reader:
            name = row.pop("Name")
            data[name] = {}

            for key, value in row.items():
                # Convert numeric values to int or float where possible.
                try:
                    if "." in value:
                        data[name][key] = float(value)
                    else:
                        data[name][key] = int(value)
                except ValueError:
                    data[name][key] = value

    return data


def main() -> None:
    """Convert all CSV files into their corresponding JSON files."""
    project_dir = Path(__file__).resolve().parent
    csv_dir = project_dir / "csv"
    json_dir = project_dir / "json"

    # Ensure the output directory exists.
    json_dir.mkdir(parents=True, exist_ok=True)

    for name in ("skills", "races", "classes"):
        csv_path = csv_dir / f"{name}.csv"
        json_path = json_dir / f"{name}.json"

        try:
            json_data = csv_to_json_dict(csv_path)

            with json_path.open("w", encoding="utf-8") as file:
                json.dump(json_data, file, indent=2, ensure_ascii=False)
                file.write("\n")

            print(f"✅ Created: {json_path.relative_to(project_dir)}")

        except (OSError, csv.Error, ValueError, KeyError) as error:
            print(f"❌ Error converting {csv_path.name}: {error}")


if __name__ == "__main__":
    main()

