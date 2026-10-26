#!/usr/bin/env python3
"""
JSON to CSV Converter (No dependencies).

Converts JSON files from the json/ directory into CSV files
in the csv/ directory using only the Python standard library.
"""

import csv
import json
from pathlib import Path
from typing import Any


def load_json(file_path: Path) -> dict[str, Any]:
    """Load a JSON file."""
    with file_path.open("r", encoding="utf-8") as file:
        return json.load(file)


def json_to_csv(json_data: dict[str, Any], output_path: Path) -> None:
    """Convert a JSON dictionary to a CSV file."""
    if not json_data:
        return

    # Collect headers from all entries to handle differing attributes.
    headers = ["Name"]
    for attributes in json_data.values():
        for key in attributes:
            if key not in headers:
                headers.append(key)

    with output_path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=headers)
        writer.writeheader()

        for name, attributes in json_data.items():
            row = {"Name": name}
            row.update(attributes)
            writer.writerow(row)


def main() -> None:
    """Convert all JSON files into their corresponding CSV files."""
    project_dir = Path(__file__).resolve().parent
    json_dir = project_dir / "json"
    csv_dir = project_dir / "csv"

    # Ensure the output directory exists.
    csv_dir.mkdir(parents=True, exist_ok=True)

    for name in ("skills", "races", "classes"):
        json_path = json_dir / f"{name}.json"
        csv_path = csv_dir / f"{name}.csv"

        try:
            data = load_json(json_path)
            json_to_csv(data, csv_path)

            print(f"✅ Created: {csv_path.relative_to(project_dir)} "
                  f"({len(data)} rows)")

        except (OSError, json.JSONDecodeError, csv.Error, ValueError,
                AttributeError, TypeError) as error:
            print(f"❌ Error processing {json_path.name}: {error}")

    print("\n🎉 Conversion complete!")


if __name__ == "__main__":
    main()

