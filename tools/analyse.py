#!/usr/bin/env python3
"""Summarise one captured run. Optional reference_depth_mm must be independently measured."""
import argparse
import csv
import json
import math
from pathlib import Path

def summarise(rows):
    samples = [r for r in rows if r['sample_event']=='1']
    valid = [r for r in samples if r['validity']=='VALID']
    errors = [int(r['depth_mm'])-float(r['reference_depth_mm']) for r in valid if r.get('reference_depth_mm','').strip()]
    summary = {'logged_rows':len(rows),'sample_events':len(samples),'valid_sample_events':len(valid),
        'valid_sample_fraction':len(valid)/len(samples) if samples else None,
        'permission_transitions':sum(a['permission']!=b['permission'] for a,b in zip(rows,rows[1:]))}
    if errors:
        summary.update(reference_comparisons=len(errors),bias_mm=sum(errors)/len(errors),
            mae_mm=sum(abs(e) for e in errors)/len(errors),rmse_mm=math.sqrt(sum(e*e for e in errors)/len(errors)),
            worst_absolute_error_mm=max(abs(e) for e in errors))
    return summary

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('csv_file',type=Path);a=p.parse_args()
    with a.csv_file.open() as f: rows=list(csv.DictReader(f))
    # Separate boots: a reset is not a permission transition within a run.
    sessions=sorted(set(r['session'] for r in rows))
    print(json.dumps({s:summarise([r for r in rows if r['session']==s]) for s in sessions},indent=2))
if __name__=='__main__':main()
