import os
import re

def fix_file_indentation(filepath):
    with open(filepath, 'r', encoding='utf-8-sig', errors='ignore') as f:
        lines = f.readlines()
    
    new_lines = []
    changed = False
    
    for i in range(len(lines)):
        line = lines[i]
        stripped = line.lstrip()
        
        # 如果这一行是注释且没有缩进
        if stripped.startswith('//') and not line.startswith((' ', '\t')):
            # 尝试寻找周围行的缩进
            indent = ""
            
            # 1. 查找下一行非空行的缩进
            for j in range(i + 1, min(i + 5, len(lines))):
                next_line = lines[j]
                if next_line.strip():
                    match = re.match(r'^(\s+)', next_line)
                    if match:
                        indent = match.group(1)
                    break
            
            # 2. 如果没找到，查找上一行缩进
            if not indent and i > 0:
                match = re.match(r'^(\s+)', lines[i-1])
                if match:
                    indent = match.group(1)
            
            if indent:
                new_lines.append(indent + stripped)
                changed = True
                continue
        
        new_lines.append(line)
        
    if changed:
        with open(filepath, 'w', encoding='utf-8-sig') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    source_dir = r"e:\MyProject\Actions\ProjectUSA\Source"
    count = 0
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.lower().endswith(('.cpp', '.h', '.cs')):
                path = os.path.join(root, file)
                if fix_file_indentation(path):
                    print(f"🔧 已修复缩进: {os.path.relpath(path, source_dir)}")
                    count += 1
    print(f"✅ 修复完成，共处理 {count} 个文件")

if __name__ == "__main__":
    main()
