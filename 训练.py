import os
import random
import shutil

# 1. 图片和标签都在同一个文件夹 dataset 里
image_dir = r'C:\Users\Lenovo\Desktop\shentong\dataset'
label_dir = r'C:\Users\Lenovo\Desktop\shentong\dataset'
# 2. 目标路径（不变）
base_dir = r'C:\Users\Lenovo\Desktop\shentong\dataset_yolo'
img_train_dir = os.path.join(base_dir, 'images', 'train')
img_val_dir = os.path.join(base_dir, 'images', 'val')
lbl_train_dir = os.path.join(base_dir, 'labels', 'train')
lbl_val_dir = os.path.join(base_dir, 'labels', 'val')

# 创建文件夹（先清空旧的，避免混入上次的数据）
if os.path.exists(base_dir):
    shutil.rmtree(base_dir)
    print(f"🗑️ 已清空旧的 {base_dir}")

for path in [img_train_dir, img_val_dir, lbl_train_dir, lbl_val_dir]:
    os.makedirs(path, exist_ok=True)

# 3. 获取所有图片列表
images = [f for f in os.listdir(image_dir) if f.endswith('.jpg')]
random.shuffle(images)

# 划分80%训练，20%验证
split_idx = int(len(images) * 0.8)
train_imgs = images[:split_idx]
val_imgs = images[split_idx:]

def move_files(file_list, img_dest, lbl_dest):
    count = 0
    for img_name in file_list:
        txt_name = img_name.replace('.jpg', '.txt')
        src_img = os.path.join(image_dir, img_name)
        src_txt = os.path.join(label_dir, txt_name)

        if os.path.exists(src_txt):
            shutil.copy(src_img, os.path.join(img_dest, img_name))
            shutil.copy(src_txt, os.path.join(lbl_dest, txt_name))
            count += 1
        else:
            print(f"⚠️ 未找到对应标签，跳过: {img_name}")
    return count

print("正在分配训练集...")
train_count = move_files(train_imgs, img_train_dir, lbl_train_dir)
print("正在分配验证集...")
val_count = move_files(val_imgs, img_val_dir, lbl_val_dir)

print(f"✅ 整理完成！训练集: {train_count} 张, 验证集: {val_count} 张")
print(f"目标文件夹: {base_dir}")