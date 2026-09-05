"""Reject geometry-changing declarations in main-window theme overrides."""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[2]
THEME = ROOT / "assets" / "ui" / "theme" / "dark.qss"

# Dark/light switching is a color operation. These declarations alter size
# hints, layout metrics or typography and belong in the shared structural QSS.
FORBIDDEN = re.compile(
    r"(?im)^\s*(?:min-|max-)?(?:width|height)\s*:|"
    r"^\s*(?:padding|margin)(?:-[a-z]+)?\s*:|"
    r"^\s*font(?:-family|-size)?\s*:"
)


def main() -> int:
    source = THEME.read_text(encoding="utf-8")
    matches = [match.group(0).strip() for match in FORBIDDEN.finditer(source)]
    if matches:
        print("THEME_COLOR_ONLY_CHECK=FAILED")
        for item in matches:
            print(f"forbidden declaration: {item}")
        return 1
    print("THEME_COLOR_ONLY_CHECK=PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
