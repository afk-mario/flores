#!/bin/bash
# Converts all .ogg files in the current directory to .wav
# Output: signed 16-bit PCM, mono or stereo

for f in *.ogg; do
  # Skip if no .ogg files are found
  [ -e "$f" ] || continue

  # Strip extension and set output name
  out="${f%.ogg}.wav"

  echo "Converting: $f -> $out"

  # Convert with ffmpeg
  ffmpeg -i "$f" -acodec pcm_s16le -ac 1 -ar 44100 "$out"
done
