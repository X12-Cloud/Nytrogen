#!/bin/bash
set -e

echo "📦 Staging all changes..."
git add .

echo "📝 Enter commit message:"
read msg

git commit -m "$msg"
git push origin main
echo "✅ Pushed to GitHub!"

