#!/bin/bash
number=`cat build_number`
let num=$((number + 1))
echo "$num" | tee build_number
