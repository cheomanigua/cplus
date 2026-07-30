#!/usr/bin/env python3
"""
CSV to JSON Converter (Reverse)

Converts skills.csv, races.csv, and classes.csv back into 
the original JSON format (skills.json, races.json, classes.json).
Uses only standard library - no pandas needed.
"""

import csv
import json
from pathlib import Path
from typing import Dict, Any

def csv_to_json_dict(csv_path: Path) -> Dict[str, Any]:
    """Convert CSV file back to original nested JSON structure."""
    data: Dict[str, Any] = {}
    
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        
        for row in reader:
            name = row.pop('Name')  # Remove Name column and use as key
            data[name] = {}
            
            for key, value in row.items():
                # Try to convert numbers back to int/float
                try:
                    if '.' in value:
                        data[name][key] = float(value)
                    else:
                        data[name][key] = int(value)
                except ValueError:
                    # Keep as string if not a number
                    data[name][key] = value
    
    return data

def main():
    """Main function to convert CSVs to JSONs."""
    script_dir = Path(__file__).parent.resolve()
    
    # Input CSVs and output JSONs
    conversions = {
        'skills': {'csv': script_dir / 'skills.csv', 'json': script_dir / 'skills.json'},
        'races':  {'csv': script_dir / 'races.csv',  'json': script_dir / 'races.json'},
        'classes':{'csv': script_dir / 'classes.csv', 'json': script_dir / 'classes.json'}
    }
    
    for name, paths in conversions.items():
        try:
            json_data = csv_to_json_dict(paths['csv'])
            
            with open(paths['json'], 'w', encoding='utf-8') as f:
                json.dump(json_data, f, indent=2)
            
            print(f"✅ Created: {paths['json'].name} from {paths['csv'].name}")
            
        except Exception as e:
            print(f"❌ Error converting {name}: {e}")

    print("\n🎉 All CSV files converted back to JSON successfully!")

if __name__ == "__main__":
    main()
