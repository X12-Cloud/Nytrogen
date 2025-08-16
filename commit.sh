#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <commit_message>"
    exit 1
fi

echo "📦 Staging all changes..."
git add .

git commit -m "$1"
git push origin main
echo "✅ Pushed to GitHub!"