#!/bin/bash
for ((i=2;i<6;i++)); do
    cp  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/pfd_cii1/*  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/pfd_cii${i}/
    cp  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/rle_cii1/*  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/rle_cii${i}/
    cp  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/epd_cii1/*  /lustre/widow2/scratch/xzou2/widow0-20130305/gts/epd_cii${i}/
done

./bmap_build_thrpt_sith.sh

./timing_encode.sh

echo "we done"
