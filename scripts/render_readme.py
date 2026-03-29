import re
import os

TEMPLATE_PATH = './docs/README.template.md'
OUTPUT_PATH = './README.md'

def resolve_includes():
    if not os.path.exists(TEMPLATE_PATH):
        print(f"Error: Template not found at {TEMPLATE_PATH}")
        return

    with open(TEMPLATE_PATH, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex for Transclusion syntax: :[Optional Title](path/to/file.ext)
    pattern = re.compile(r':\[.*?\]\((.*?)\)')

    def replace_match(match):
        file_path = match.group(1)
        base_dir = os.path.dirname(TEMPLATE_PATH)
        full_path = os.path.normpath(os.path.join(base_dir, file_path))
        
        # Get extension for the code fence (e.g., 'json', 'yaml')
        ext = os.path.splitext(full_path)[1].lstrip('.')
        
        try:
            with open(full_path, 'r', encoding='utf-8') as snippet:
                file_content = snippet.read().strip()
                # Wrap in code fences automatically
                return f"```{ext}\n{file_content}\n```"
        except FileNotFoundError:
            return f""

    processed_content = pattern.sub(replace_match, content)

    with open(OUTPUT_PATH, 'w', encoding='utf-8') as f:
        f.write(processed_content)
    
    print(f"Successfully sync'd {OUTPUT_PATH}")

if __name__ == "__main__":
    resolve_includes()