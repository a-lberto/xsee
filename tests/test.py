import subprocess
import json
import pytest
from pathlib import Path

# Configuration
TESTS_DIR = Path(__file__).parent 
PROJECT_ROOT = TESTS_DIR.parent

ENGINES = {
    "python": ["uv", "run", str(PROJECT_ROOT / "engines/python/src/main.py")],
    "nodejs": ["node", str(PROJECT_ROOT / "engines/js/src/index.js")],
    "cpp": [str(PROJECT_ROOT / "engines/cpp/build/xsee")]
}

def get_test_scenarios():
    """Finds all subdirectories in /tests that have an expected.json."""
    return [d for d in TESTS_DIR.iterdir() if d.is_dir() and (d / "expected.json").exists()]

@pytest.mark.parametrize("scenario_path", get_test_scenarios(), ids=lambda p: f"Scenario: {p.name}")
@pytest.mark.parametrize("engine_name", ENGINES.keys())
def test_engine_output(scenario_path, engine_name):
    # Setup Paths
    input_html = scenario_path / "input.html"
    schema_yaml = scenario_path / "xsee.yaml"
    expected_path = scenario_path / "expected.json"
    
    # Engine command adjustment
    cmd = ENGINES[engine_name].copy()
    if engine_name == "cpp" and Path(cmd[0] + ".exe").exists():
        cmd[0] += ".exe"

    # Execution
    result = subprocess.run(
        [*cmd, str(input_html), "--yaml", str(schema_yaml)],
        capture_output=True,
        text=True,
        encoding="utf-8"
    )

    # Check for execution errors with descriptive messages
    assert result.returncode == 0, (
        f"Engine '{engine_name}' crashed!\n"
        f"STDOUT: {result.stdout}\n"
        f"STDERR: {result.stderr}"
    )

    # Parse Data
    try:
        actual_data = json.loads(result.stdout)
    except json.JSONDecodeError:
        pytest.fail(f"Engine '{engine_name}' output invalid JSON: {result.stdout}")

    expected_data = json.loads(expected_path.read_text(encoding="utf-8"))
    
    # Compare JSON
    assert actual_data == expected_data