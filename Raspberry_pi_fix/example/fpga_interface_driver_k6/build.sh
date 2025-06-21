#!/bin/bash
echo "🛠 Building drivers and app..."
make clean
make
echo "✅ Build complete."

echo "📦 Moving modules..."
mkdir -p /home/kjh/Modules
cp *.ko /home/kjh/Modules
echo "✅ Modules ready in /home/kjh/Modules"

