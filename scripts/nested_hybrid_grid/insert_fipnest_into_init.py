#!/usr/bin/env python3
"""Insert the nested-hybrid-grid parent-child arrays into an Eclipse INIT file.

Reads the FIPNEST/FIPSLOT/REFINE keywords from a GRDECL file (typically the
"<gridbase>_FIPNEST.grdecl" sidecar that ResInsight auto-exports when importing a
nested hybrid grid through the OLDIJK sidecars, see issue #14510) and appends them
as INTE arrays to a copy of an existing INIT file. ResInsight detects FIPNEST in
the INIT file and reconstructs the nested hybrid grid without any sidecar files.

Example:
    python insert_fipnest_into_init.py DROGON_NESTED_FIPNEST.grdecl \\
        DROGON_NESTED.INIT /tmp/staged/DROGON_NESTED.INIT
"""

import argparse
import sys

import numpy as np
import resfo

DEFAULT_KEYWORDS = ["FIPNEST", "FIPSLOT", "REFINE"]


def read_grdecl_int_keywords(path, wanted):
    """Parse integer keywords from a GRDECL text file.

    Handles '--' comments and 'N*V' run-length encoded values. Returns a dict
    keyword -> list of ints for the keywords in `wanted` that are present.
    """
    wanted = {kw.upper() for kw in wanted}
    result = {}
    current = None
    values = []

    with open(path, "r", encoding="ascii", errors="replace") as f:
        for line in f:
            comment = line.find("--")
            if comment >= 0:
                line = line[:comment]

            for token in line.split():
                if current is None:
                    if token.upper() in wanted:
                        current = token.upper()
                        values = []
                    continue

                if token == "/" or token.endswith("/"):
                    token = token[:-1]
                    if token:
                        values.extend(_expand_token(token))
                    result[current] = values
                    current = None
                    continue

                values.extend(_expand_token(token))

    if current is not None:
        raise ValueError(f"keyword {current} in {path} is not terminated with '/'")
    return result


def _expand_token(token):
    if "*" in token:
        count, value = token.split("*", 1)
        return [int(value)] * int(count)
    return [int(token)]


def grid_cell_count_from_init(contents):
    """NX*NY*NZ from the INTEHEAD array (items 9-11, 1-based) of an INIT file."""
    for kw, array in contents:
        if kw.strip() == "INTEHEAD":
            return int(array[8]) * int(array[9]) * int(array[10])
    return None


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "grdecl", help="GRDECL file holding the FIPNEST/FIPSLOT/REFINE keywords"
    )
    parser.add_argument("init_in", help="existing INIT file to augment")
    parser.add_argument(
        "init_out", help="augmented INIT file to write (must differ from init_in)"
    )
    parser.add_argument(
        "--keywords",
        nargs="+",
        default=DEFAULT_KEYWORDS,
        help=f"keywords to transfer (default: {' '.join(DEFAULT_KEYWORDS)})",
    )
    parser.add_argument(
        "--formatted",
        action="store_true",
        help="write a formatted (ASCII) INIT file instead of an unformatted one",
    )
    args = parser.parse_args()

    if args.init_in == args.init_out:
        parser.error("init_out must differ from init_in")

    arrays = read_grdecl_int_keywords(args.grdecl, args.keywords)
    missing = [kw for kw in args.keywords if kw.upper() not in arrays]
    if missing:
        sys.exit(f"error: keyword(s) {', '.join(missing)} not found in {args.grdecl}")

    contents = [(kw, array) for kw, array in resfo.read(args.init_in)]

    cell_count = grid_cell_count_from_init(contents)
    if cell_count is not None:
        for kw in args.keywords:
            n = len(arrays[kw.upper()])
            if n != cell_count:
                sys.exit(
                    f"error: {kw} has {n} values, the INIT grid has {cell_count} cells (NX*NY*NZ)"
                )

    # Replace any existing occurrences, then append in the requested order.
    upper_keywords = {kw.upper() for kw in args.keywords}
    contents = [
        (kw, array) for kw, array in contents if kw.strip() not in upper_keywords
    ]
    for kw in args.keywords:
        contents.append(
            (kw.upper().ljust(8), np.asarray(arrays[kw.upper()], dtype=np.int32))
        )

    file_format = resfo.Format.FORMATTED if args.formatted else resfo.Format.UNFORMATTED
    resfo.write(args.init_out, contents, fileformat=file_format)

    print(
        f"wrote {args.init_out} with {', '.join(kw.upper() for kw in args.keywords)} ({len(contents)} arrays total)"
    )


if __name__ == "__main__":
    main()
