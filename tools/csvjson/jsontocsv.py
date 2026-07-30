#!/usr/bin/env python3
"""
JSON to CSV Converter (No dependencies)

Converts skills.json, races.json, and classes.json 
(assumed to be in the same directory as this script) 
into clean CSV files using only standard library.
"""

import json
import csv
from pathlib import Path
from typing import Dict, Any

def load_json(file_path: str) -> Dict[str, Any]:
    """Load JSON file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def json_to_csv(json_data: Dict[str, Any], output_path: Path):
    """Convert JSON dict to CSV file."""
    if not json_data:
        return
    
    # Get all keys for headers
    first_item = next(iter(json_data.values()))
    headers = ['Name'] + list(first_item.keys())
    
    with open(output_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=headers)
        writer.writeheader()
        
        for name, attributes in json_data.items():
            row = {'Name': name}
            row.update(attributes)
            writer.writerow(row)

def main():
    """Main conversion function."""
    script_dir = Path(__file__).parent.resolve()
    
    # Input files
    input_files = {
        'skills': script_dir / 'skills.json',
        'races': script_dir / 'races.json',
        'classes': script_dir / 'classes.json'
    }
    
    output_dir = script_dir / 'game_data_csv'
    output_dir.mkdir(exist_ok=True)
    
    for name, json_path in input_files.items():
        try:
            data = load_json(json_path)
            output_path = output_dir / f"{name}.csv"
            
            json_to_csv(data, output_path)
            print(f"✅ Created: {output_path.name} ({len(data)} rows)")
            
        except Exception as e:
            print(f"❌ Error processing {name}: {e}")

    print(f"\n🎉 Conversion complete! Files saved to ./{output_dir.name}/")

if __name__ == "__main__":
    main()
