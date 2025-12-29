#!/bin/bash

# Create OBJ/MTL Files
OUTPUT=$(/Applications/Blender.app/Contents/MacOS/Blender --background --python /Users/vivek/raytracer2/scripts/create_scene.py 2>&1)
OBJ_PATH=$(echo "$OUTPUT" | grep "^OBJ file:" | awk '{print $3}')

if [ -z "$OBJ_PATH" ]; then
  echo "Error: Failed to extract OBJ path"
  echo "$OUTPUT"
  exit 1
fi
echo "Rendering: $OBJ_PATH"
~/raytracer2/build/apps/render_to_ppm "$OBJ_PATH"