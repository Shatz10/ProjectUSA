import os
import json
import re
import time
import pandas as pd
from deep_translator import GoogleTranslator
from typing import Optional, Dict, Any, List, Union
import argparse
import sys

# ================= 配置区 =================
# 默认语言映射 (程序内部使用的语言代码 -> Google翻译代码)
DEFAULT_LANG_MAP = {
    'zhcn': 'zh-CN',
    'zhtw': 'zh-TW',
    'en': 'en',
    'ge': 'de',
    'fr': 'fr',
    'ru': 'ru',
    'pt': 'pt',
    'es': 'es',
    'ko': 'ko',
    'ja': 'ja',
    'it': 'it'
}

GLOSSARY_FILES = [
    'translate/Glossary_Completed.csv',
    'translate/Glossary.csv',
    'Glossary.csv'
]

# 常见代码文件后缀，翻译时会更谨慎处理
CODE_EXTENSIONS = {'.h', '.cpp', '.cs', '.c', '.py', '.js', '.ts', '.lua', '.gradle', '.xml'}

RETRY_COUNT = 3
# ==========================================

class FolderTranslator:
    def __init__(self, target_lang: str, source_lang: str = 'zh-CN', use_glossary: bool = True):
        self.target_lang = DEFAULT_LANG_MAP.get(target_lang.lower(), target_lang)
        self.source_lang = source_lang
        self.use_glossary = use_glossary
        self.glossary_df = None
        self.source_col = None
        
        # 规范化源语言代码，用于检测和术语表列名
        self.norm_source = source_lang.lower().replace('-', '')
        if self.norm_source == 'zhcn': self.norm_source = 'zh'
        
        if use_glossary:
            self.load_glossary()

    def contains_source_language(self, text: str) -> bool:
        """根据配置的源语言检查文本是否包含该语言字符"""
        if not text or not isinstance(text, str):
            return False
            
        # 中文检测 (简体/繁体)
        if 'zh' in self.norm_source:
            return any('\u4e00' <= char <= '\u9fff' for char in text)
        
        # 韩文检测
        if 'ko' in self.norm_source:
            # \uac00-\ud7af: 韩文音节; \u1100-\u11ff: 韩文字母; \u3130-\u318f: 韩文兼容字母
            return any('\uac00' <= char <= '\ud7af' or 
                       '\u1100' <= char <= '\u11ff' or 
                       '\u3130' <= char <= '\u318f' for char in text)
        
        # 日文检测
        if 'ja' in self.norm_source:
            return any('\u3040' <= char <= '\u309f' or  # 平假名
                       '\u30a0' <= char <= '\u30ff' or  # 片假名
                       '\u4e00' <= char <= '\u9fff' for char in text)  # 汉字
        
        # 默认回退：如果有任何非 ASCII 字符，则认为可能需要翻译
        return any(ord(char) > 127 for char in text)

    def load_glossary(self):
        """加载术语表并排序（长词优先，防止误切）"""
        for path in GLOSSARY_FILES:
            if os.path.exists(path):
                try:
                    df = pd.read_csv(path)
                    
                    # 寻找匹配源语言的列
                    source_col = None
                    possible_cols = [self.norm_source, self.source_lang, 'zh', 'ko', 'source']
                    for col in possible_cols:
                        if col in df.columns:
                            source_col = col
                            break
                    
                    if not source_col:
                        # 如果没有精确匹配，且只有一列，尝试第一列作为源
                        if len(df.columns) >= 2:
                            source_col = df.columns[0]
                        else:
                            print(f"⚠️ 警告: 术语表 {path} 中缺少源语言列，跳过")
                            continue
                    
                    self.source_col = source_col
                    df['len'] = df[source_col].astype(str).str.len()
                    self.glossary_df = df.sort_values('len', ascending=False)
                    print(f"📖 已加载术语表: {path} (源语言列: {source_col})")
                    return
                except Exception as e:
                    print(f"⚠️ 加载术语表 {path} 失败: {e}")
        
        if self.use_glossary:
            print("⚠️ 未找到有效的术语表或格式不匹配，将不使用术语保护")
            self.use_glossary = False

    def translate_text(self, text: str) -> str:
        """翻译文本，带缩进保护和术语保护"""
        if not text or not str(text).strip():
            return text
            
        # 1. 提取并保留前导空格/缩进
        match = re.match(r'^(\s*)', text)
        leading_ws = match.group(1) if match else ""
        content_to_translate = text[len(leading_ws):].rstrip()
        
        # 2. 如果除去缩进后的内容不包含源语言字符，直接返回原文
        if not self.contains_source_language(content_to_translate):
            return text
            
        # 3. 针对代码注释符号的额外保护 (避免 // 被翻译成 / / 或丢失)
        prefix = ""
        if content_to_translate.startswith("//"):
            prefix = "//"
            content_to_translate = content_to_translate[2:]
        elif content_to_translate.startswith("/*"):
            prefix = "/*"
            content_to_translate = content_to_translate[2:]
            
        protected_text = content_to_translate
        matched_terms = {}
        
        # 4. 占位符替换术语 (如果有术语表)
        if self.use_glossary and self.glossary_df is not None:
            for i, row in self.glossary_df.iterrows():
                term = str(row[self.source_col])
                if not term or term == 'nan' or pd.isna(term) or term.strip() == "":
                    continue
                
                # 获取翻译，尝试目标语言列
                target_val = term
                for col in [self.target_lang, self.target_lang.split('-')[0]]:
                    if col in row and pd.notna(row[col]):
                        target_val = str(row[col])
                        break
                
                if term in protected_text:
                    placeholder = f"###{i}###"
                    protected_text = protected_text.replace(term, placeholder)
                    matched_terms[placeholder] = target_val
        
        # 4. 调用翻译
        translated = self.call_google_translate(protected_text)
        
        # 5. 还原术语
        if matched_terms:
            for ph, target_val in matched_terms.items():
                term_id = ph.strip("#")
                # 匹配占位符，忽略翻译过程中可能产生的空格
                pattern = re.compile(r'###\s*' + re.escape(term_id) + r'\s*###', re.IGNORECASE)
                translated = pattern.sub(target_val, translated)
        
        # 6. 重新拼接缩进和前缀
        return leading_ws + prefix + translated

    def call_google_translate(self, text: str) -> str:
        """带重试机制的翻译调用"""
        for attempt in range(RETRY_COUNT):
            try:
                result = GoogleTranslator(source=self.source_lang, target=self.target_lang).translate(text)
                if result:
                    return result
                return text
            except Exception as e:
                if attempt < RETRY_COUNT - 1:
                    wait_time = (attempt + 1) * 2
                    print(f"⚠️ 翻译中继错误，{wait_time}秒后重试: {e}")
                    time.sleep(wait_time)
                else:
                    print(f"❌ 翻译彻底失败（已重试{RETRY_COUNT}次）: {e}")
                    return text
        return text

    def process_json(self, data: Any) -> Any:
        """递归递归翻译 JSON 内容"""
        if isinstance(data, dict):
            return {k: self.process_json(v) for k, v in data.items()}
        elif isinstance(data, list):
            return [self.process_json(i) for i in data]
        elif isinstance(data, str):
            if self.contains_source_language(data):
                return self.translate_text(data)
            return data
        else:
            return data

    def translate_file(self, input_path: str, output_path: str = None, in_place: bool = False):
        """翻译单个文件"""
        if in_place:
            output_path = input_path
        elif not output_path:
            raise ValueError("Must provide output_path if not in_place")

        ext = os.path.splitext(input_path)[1].lower()
        
        # 尝试多种编码读取文件
        encodings = ['utf-8-sig', 'utf-8', 'gbk', 'utf-16']
        content = None
        used_encoding = 'utf-8'
        
        for enc in encodings:
            try:
                with open(input_path, 'r', encoding=enc) as f:
                    content = f.read()
                used_encoding = enc
                break
            except UnicodeDecodeError:
                continue
            except Exception as e:
                print(f"⚠️ 读取 {input_path} 时出错 ({enc}): {e}")
                
        if content is None:
            print(f"❌ 错误: 无法识别文件编码: {input_path}")
            return

        try:
            if ext == '.json':
                data = json.loads(content)
                translated_data = self.process_json(data)
                with open(output_path, 'w', encoding='utf-8') as f:
                    json.dump(translated_data, f, ensure_ascii=False, indent=4)
            else:
                lines = content.splitlines()
                translated_lines = []
                for line in lines:
                    # 仅翻译包含源语言的行，保护纯代码行
                    if self.contains_source_language(line):
                        translated_lines.append(self.translate_text(line.rstrip()) + '\n')
                    else:
                        translated_lines.append(line + '\n')
                
                with open(output_path, 'w', encoding='utf-8') as f:
                    f.writelines(translated_lines)
            
            print(f"✅ 已完成: {os.path.basename(input_path)} (编码: {used_encoding})")
        except Exception as e:
            print(f"❌ 处理文件 {input_path} 时出错: {e}")

    def translate_folder(self, input_dir: str, output_dir: str = None, extensions: List[str] = None, in_place: bool = False):
        """翻译整个文件夹"""
        if not in_place and output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir)
            print(f"📁 已创建输出目录: {output_dir}")
        
        for root, dirs, files in os.walk(input_dir):
            target_root = None
            rel_path = os.path.relpath(root, input_dir)
            
            if not in_place and output_dir:
                target_root = os.path.join(output_dir, rel_path)
                if not os.path.exists(target_root):
                    os.makedirs(target_root)
            
            for file in files:
                ext = os.path.splitext(file)[1].lower()
                if extensions and ext not in extensions:
                    continue
                
                input_path = os.path.join(root, file)
                output_path = os.path.join(target_root, file) if target_root else input_path
                
                print(f"🚀 正在处理: {os.path.normpath(os.path.join(rel_path, file))}")
                self.translate_file(input_path, output_path, in_place=in_place)
                # 避免触发频率限制
                time.sleep(0.05)

def main():
    parser = argparse.ArgumentParser(description="多语言文件夹翻译工具 (支持韩文/中文/日文)")
    parser.add_argument("--input", "-i", help="源文件夹路径")
    parser.add_argument("--output", "-o", help="输出文件夹路径 (默认: input_translated)")
    parser.add_argument("--lang", "-l", default="en", help="目标语言代码 (默认: en)")
    parser.add_argument("--source", "-s", default="ko", help="源语言代码 (如: zh-CN, ko, ja, 默认: ko)")
    parser.add_argument("--ext", "-e", help="限定文件后缀，多个用逗号分隔 (如: .json,.cpp,.h)")
    parser.add_argument("--no-glossary", action="store_true", help="禁用术语表保护")
    parser.add_argument("--in-place", "-p", action="store_true", help="直接修改源文件 (慎用，建议先备份)")

    args = parser.parse_args()

    # 如果没提供输入，尝试查找 Source 文件夹
    input_dir = args.input
    if not input_dir:
        default_source = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "Source"))
        if os.path.isdir(default_source):
            input_dir = default_source
        else:
            parser.print_help()
            sys.exit(1)

    if not os.path.isdir(input_dir):
        print(f"❌ 错误: 输入路径不存在: {input_dir}")
        sys.exit(1)
    
    output_dir = None
    if not args.in_place:
        output_dir = args.output or f"{input_dir.rstrip(os.sep)}_translated"
    
    extensions = None
    if args.ext:
        extensions = [e.strip().lower() if e.startswith('.') else f".{e.strip().lower()}" for e in args.ext.split(',')]
    
    print(f"🌟 开始文件夹翻译任务")
    print(f"📂 源目录: {input_dir}")
    if args.in_place:
        print(f"⚠️ 模式: 直接修改源文件 (In-place)")
    else:
        print(f"📂 输出目录: {output_dir}")
    print(f"🌐 翻译方向: {args.source} -> {args.lang}")
    print(f"🛠️ 过滤类型: {extensions if extensions else '全部'}")
    print("-" * 50)

    translator = FolderTranslator(
        target_lang=args.lang, 
        source_lang=args.source, 
        use_glossary=not args.no_glossary
    )
    
    translator.translate_folder(input_dir, output_dir, extensions, in_place=args.in_place)
    
    print("-" * 50)
    if args.in_place:
        print(f"✨ 翻译完成！源文件已更新。")
    else:
        print(f"✨ 翻译完成！输出保存在: {output_dir}")

if __name__ == "__main__":
    main()

