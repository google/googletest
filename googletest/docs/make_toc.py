"""
Description
------------
Adds a Table of Contents(TOC) to a .md
    * First, matches to a title (=line starting with '#').
    * Second, matches each subsequent title and makes a link to it

The title is expected at level '#' or '##'
Subsequent titles are indexed if between '#' and '###'

If file already has a TOC, does nothing. You can regenerate TOC for an updated document by passing --force.

Known limitation
------------------
If all non-first titles are '###' or more, TOC will be over-indented. Then needs to be manually dedented.

Author
-------
Brice Letcher
"""

import sys
import re

if len(sys.argv) == 1:
    print(f"Usage: {sys.argv[0]} (file.md)+ [--force]")
    exit(0)

force = True if sys.argv[-1] == "--force" else False
if force:
    sys.argv = sys.argv[:-1]

SPECIAL_CHARS = '[`()*#?!{}:,./"]'
TOC_HEADER = "## Contents"


for fname in sys.argv[1:]:
    print(fname)
    with open(fname) as f_in:
        full_file = f_in.read()

    if re.search(TOC_HEADER, full_file) is not None:
        if not force:
            print(
                f"Found {TOC_HEADER}, nothing to do. To regenerate, run with --force."
            )
            continue
        else:  # Case: remove existing toc
            toc_match = re.search(
                f"{TOC_HEADER}.*?\n\n", full_file, re.DOTALL
            )  # *? is minimal match
            full_file = full_file[: toc_match.start()] + full_file[toc_match.end() :]

    title_match = re.search("^#{1,2} .*", full_file, re.MULTILINE)

    toc_headers = list()
    for match in re.findall("^#{1,3} .*", full_file[title_match.end() :], re.MULTILINE):
        num_hash = match.split()[0].count("#")
        link = match.replace(f'{"#"*num_hash} ', "#")
        title = f"[{link[1:]}]"
        link = "#" + re.sub(SPECIAL_CHARS, "", link[1:])
        link = link.replace(" ", "-").lower()
        link = f"({link})"
        indent = "\t" * (num_hash - 2)
        header = f"{indent}1. {title}{link}"
        toc_headers.append(header)

    result = (
        full_file[: title_match.start()]
        + full_file[title_match.start() : title_match.end()]
        + "\n\n"
        + TOC_HEADER
        + "\n"
        + "\n".join(toc_headers)
        + full_file[title_match.end() :]
    )

    with open(fname, "w") as f_out:
        f_out.write(result)
