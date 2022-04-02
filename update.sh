#!/bin/bash

git remote add ref git@github.com:CS429-S2022/SE-Lab-Student.git
git config pull.rebase false
git pull --allow-unrelated-histories --set-upstream ref main --no-commit
git commit -m "pulled weekly checkpoint"
git push -u origin main