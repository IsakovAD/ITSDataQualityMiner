# ITS Async QA (`its-qa-qc`)

A tool for comparing ALICE ITS Quality Control objects across two different
processing passes (e.g. `apass4` vs Monte Carlo, or `cpass0` vs `apass1`) and
flagging objects whose ratio deviates beyond a threshold. Problematic objects
are collected into a single multi-page PDF report — old pass, new pass, and
their ratio side by side — so a shifter can scan a whole period at a glance
instead of opening hundreds of objects by hand.

It is one of two executables in the ITSDataQualityMiner project; the other,
`its-dqm`, downloads QC objects around ITS stave dead periods. This document
covers `its-qa-qc` only.

## What it does

For every run in a run list, and every enabled object in the configuration, the
tool:

1. Resolves the object's CCDB timestamp from a local SQLite cache (see
   [Timestamp cache](#timestamp-cache)), refreshing the cache from CCDB on
   startup.
2. Downloads the object for both the *old* and *new* pass.
3. Optionally normalizes by the number of readout frames and applies
   object-specific rebinning.
4. Builds the ratio (old / new) and checks whether its minimum or maximum
   deviates from unity by more than the threshold.
5. If it does, draws old / new / ratio into the PDF report and records the
   deviation and the coordinate where it occurs.

## Requirements

The ALICE O2 + QualityControl environment must be loaded before building or
running:

```bash
alienv enter O2/latest QualityControl/latest CMake/latest
```

The build links against O2, QualityControl, ROOT, FairLogger, FairMQ, Boost,
Microsoft.GSL, JAliEn-ROOT and SQLite3 — all provided by the alienv stack.

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . -- -j$(nproc)
```

This produces `its-qa-qc` (along with the other project executables). See the
top-level project README for CMake troubleshooting specific to the O2 stack.

## Running

The tool resolves input and output paths relative to the **current working
directory**, so run it from the project root:

```bash
cd <project-root>
./build/its-qa-qc --input analysis_params.json
```

Options:

| Flag | Default | Meaning |
|------|---------|---------|
| `--input`, `-i` | *(required)* | analysis parameters JSON (see below) |
| `--output`, `-o` | `../downloads` | output directory hint |
| `--help`, `-h` | | print usage and exit |

The deviation threshold (fraction away from 1.0 that counts as "problematic")
is currently set in `apps/AssyncQA.cxx` when the processor is constructed
(default `0.05`, i.e. 5%).

## Configuration

All configuration lives under `inputs/its-qa-qc/`.

### Analysis parameters

The file passed to `--input` selects the two passes to compare and the run
list. Example (`analysis_params.json`):

```json for MC vs async
{
  "run_list": "runs_O2_6820_LHC23k4i_apass4-LHC23.txt",
  "DataType_old": "qc_async",
  "DataType_new": "qc_mc",
  "DataPass_old": "apass4",
  "DataPass_new": "passMC",
  "MCPeriod_old": " ",
  "MCPeriod_new": " LHC23k4i "
}
```

```json for data vs async
{
  "run_list": "runs_O2_6820_LHC23k4i_apass4-LHC23.txt",
  "DataType_old": "qc_async",
  "DataType_new": "qc",
  "DataPass_old": "apass4",
  "DataPass_new": " ",
  "MCPeriod_old": " ",
  "MCPeriod_new": " "
}
```


`DataType_*` selects the CCDB database and the matching object list
(`objects_<DataType>.json`). Recognized types: `qc_async`, `qc`, `qc_mc`.

### Run list

A plain text file under `inputs/its-qa-qc/`, one run per line. The first six
characters of each line are taken as the run number; anything after is ignored,
so annotated lists work:

```
535069 - -- LHC23f
535084 -
535345 - -- LHC23g
```

### Object list

`objects_<DataType>.json` describes which objects to compare. Each entry is a
flat block of key/value pairs:

```json
{
  "isEnabled": "1",
  "Task": "Tracks",
  "Path": "AngularDistribution",
  "ObjectType": "TH2",
  "isDoROF_norm": "0",
  "isLogy": "0",
  "isLogx": "0"
}
```

| Key | Meaning |
|-----|---------|
| `isEnabled` | `1` to compare this object, `0` to skip it |
| `Task` | QC task the object belongs to (`Tracks`, `Clusters`, …); maps to the CCDB module name per database type |
| `Path` | object path within the task; a `Path` containing `avg` triggers the per-stave average-cluster builder |
| `ObjectType` | `TH1`, `TH2`, or `TEfficiency` |
| `isDoROF_norm` | normalize by number of readout frames |
| `isLogy` / `isLogx` | log axis in the report |
| `isCentralBarrelCut` | restrict ratio analysis to the central η window |

An object is compared only if it is enabled **and** a matching `Path` exists in
the *old* object list; if no match is found, the object is reported with the new
pass only.

### Module config

`inputs/db_module.conf` maps each database type and module to a CCDB path used
to enumerate timestamps. Format: `Type Module PathForTimestamps`.

```
qc_async tracks   /ITS/MO/Tracks/NClustersPerTrackEta
qc_async clusters /ITS/MO/Clusters/Layer0/ClusterOccupation
qc_mc    simulation /ITS/MO/TracksMc/efficiency_phi
```

## Timestamp cache

Resolving a CCDB object requires its validity timestamp, and querying CCDB for
every object would be slow. Instead the tool keeps a local SQLite cache per
database type at `inputs/database/db_<DataType>.db`, mapping
`(run, pass, module) → validity`.

On startup the cache is refreshed incrementally: the tool walks the CCDB object
listing newest-upload-first and stops once it reaches uploads already seen.
Reprocessed runs are handled by keeping the entry with the most recent creation
time, so a later, better reprocessing always supersedes an older one.

The `.db` files are regenerable from CCDB and should not be committed.

## Output

For each input, a folder is created at
`output/its-qa-qc/<run_list_stem>/` containing:

- `report.pdf` — the multi-page comparison report (only problematic objects)
- `Report.txt` — per-run counts of empty and problematic objects
- `<run_list_stem>_out.txt` — the list of processed runs

## Known limitations

- Paths are resolved relative to the working directory; run from the project
  root.
- The deviation threshold is compiled in rather than configurable on the
  command line.
- Histograms handed to the report are intentionally not freed until the program
  exits (ROOT draws them lazily at PDF-print time); memory grows with the number
  of problematic objects in a single invocation.
