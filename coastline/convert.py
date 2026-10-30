import json
import struct

INPUT = "east_asia.geojson"
OUTPUT = "east_asia.map"


def write_line(out, coordinates):
    if len(coordinates) < 2:
        return False

    out.write(struct.pack("<I", len(coordinates)))

    for x, y in coordinates:
        out.write(struct.pack("<ff", float(x), float(y)))

    return True


with open(INPUT, "r", encoding="utf-8") as f:
    data = json.load(f)


lines = []

for feature in data["features"]:

    geometry = feature.get("geometry")

    if geometry is None:
        continue

    geometry_type = geometry["type"]
    coordinates = geometry["coordinates"]

    if geometry_type == "LineString":

        if len(coordinates) >= 2:
            lines.append(coordinates)

    elif geometry_type == "MultiLineString":

        for line in coordinates:
            if len(line) >= 2:
                lines.append(line)


with open(OUTPUT, "wb") as out:

    # Magic
    out.write(b"MAP2")

    # Version
    out.write(struct.pack("<I", 1))

    # Number of line strings
    out.write(struct.pack("<I", len(lines)))

    for line in lines:
        write_line(out, line)


print(f"Input features: {len(data['features'])}")
print(f"Output lines:   {len(lines)}")
print(f"Written:        {OUTPUT}")
