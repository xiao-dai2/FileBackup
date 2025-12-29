# ============== 1. 创建基础目录结构 ==============
mkdir -p test_backup_dir/{docs,media,code,secret}

# ============== 2. 生成 docs 目录测试文件 ==============
# 普通文本文件（含内容）
echo "这是测试文本文件1：用于备份基础功能测试" > test_backup_dir/docs/test1.txt
# 标记文件
echo "# 测试Markdown文件\n- 备份测试项1：目录递归备份\n- 备份测试项2：小文件备份" > test_backup_dir/docs/test2.md
# 空文件（测试空文件备份）
touch test_backup_dir/docs/empty_file.txt

# ============== 3. 生成 media 目录测试文件 ==============
# 生成模拟图片文件（二进制随机内容，100KB）
dd if=/dev/urandom of=test_backup_dir/media/test_img.jpg bs=1024 count=100 > /dev/null 2>&1
# 生成大文件（50MB，测试压缩/打包效率）
dd if=/dev/zero of=test_backup_dir/media/large_file.dat bs=1M count=50 > /dev/null 2>&1

# ============== 4. 生成 code 目录测试文件 ==============
# 测试脚本文件
echo "#!/bin/bash\necho '测试脚本：备份后可执行性验证'" > test_backup_dir/code/test_script.sh
chmod +x test_backup_dir/code/test_script.sh  # 添加可执行权限
# 测试代码文件
echo "#include <iostream>\nint main() { return 0; }" > test_backup_dir/code/test_code.cpp

# ============== 5. 生成 secret 目录测试文件（加密测试） ==============
echo "测试加密备份：用户名=test, 密码=123456" > test_backup_dir/secret/password.txt

# ============== 6. 验证目录创建完成 ==============
echo "测试目录创建完成！目录结构："
tree test_backup_dir/  # 若未安装tree，执行：apt install tree -y 后再运行