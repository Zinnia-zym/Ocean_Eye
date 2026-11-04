import cv2
import numpy as np
import onnxruntime as ort
from ultralytics import YOLO
import time

# ====== 路径配置（相对路径，公开仓库友好）======
pt_path = r'models/best.pt'
onnx_path = r'models/best.onnx'
img_path = r'captures/test.jpg'

# 1. 加载原版 PyTorch 模型
pt_model = YOLO(pt_path)

# 2. 加载导出的 ONNX 模型
session = ort.InferenceSession(onnx_path, providers=['CPUExecutionProvider'])
input_name = session.get_inputs()[0].name

# 读取测试图片
img = cv2.imread(img_path)
if img is None:
    print("❌ 找不到测试图片，请修改 img_path 为真实存在的图片路径")
    exit()

# ================= 测速 1: PyTorch =================
t1 = time.time()
pt_results = pt_model(img, imgsz=640, conf=0.15)
pt_time = (time.time() - t1) * 1000
pt_boxes = pt_results[0].boxes

print(f"✅ PyTorch 推理耗时: {pt_time:.2f} ms")
print(f"   检测到目标数量: {len(pt_boxes)}")
if len(pt_boxes) > 0:
    print(f"   第一个框类别: {pt_model.names[int(pt_boxes[0].cls)]}, 置信度: {float(pt_boxes[0].conf):.3f}")


# ================= 测速 2: ONNX =================
# 预处理
input_img = cv2.resize(img, (640, 640))
input_img = cv2.cvtColor(input_img, cv2.COLOR_BGR2RGB)
input_img = input_img.transpose(2, 0, 1) # HWC -> CHW
input_tensor = np.expand_dims(input_img, axis=0).astype(np.float32) / 255.0

t2 = time.time()
outputs = session.run(None, {input_name: input_tensor})
onnx_time = (time.time() - t2) * 1000

print(f"\n✅ ONNX 推理耗时: {onnx_time:.2f} ms")
print(f"   输出张量形状: {outputs[0].shape}")
print("   (注意：ONNX直接输出的是裸张量，上面 PyTorch 的框是经过 NMS 后的，所以数量不一样是正常的。)")