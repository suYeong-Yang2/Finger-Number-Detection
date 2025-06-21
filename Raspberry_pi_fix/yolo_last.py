from ultralytics import YOLO
import subprocess
import time

# YOLO 모델 로드
model = YOLO("best_fixed.onnx")

# 클래스 매핑 (YOLO 클래스명 -> 디바이스 및 값)
class_map = {
    'dev1': ('dot_matrix', 1),
    'dev2': ('led', 2),
    'dev3': ('text_lcd', 3),
    'dev4': ('fnd', 4),
    'off': ('buzzer', 0)
}

# 이전에 활성화된 디바이스 정보
prev_device = None

# YOLO 추론 실행
for result in model.predict(source=0, show=True, stream=True):
    if len(result.boxes) > 0:
        cls_id = int(result.boxes.cls[0].item())
        class_name = model.names[cls_id]

        if class_name in class_map:
            device_type, value = class_map[class_name]

            # 디바이스 변경 시 이전 디바이스 초기화
            if prev_device != device_type:
                if prev_device == 'dot_matrix':
                    subprocess.run(["/home/kjh/Modules/fpga_test_dot", "0"])
                    #print("초기화: dot matrix off")
                elif prev_device == 'led':
                    subprocess.run(["/home/kjh/Modules/fpga_test_led", "0"])
                    #print("초기화: led off")
                elif prev_device == 'text_lcd':
                    subprocess.run(["/home/kjh/Modules/fpga_test_text_lcd", " ", "0"])
                    #print("초기화: text lcd clear")
                elif prev_device == 'fnd':
                    subprocess.run(["/home/kjh/Modules/fpga_test_fnd", "0"])
                    #print("초기화: fnd off")
                # buzzer는 off 필요 없음 (원하면 추가 가능)

                prev_device = device_type

            # 현재 디바이스 실행
            if device_type == 'dot_matrix':
                #print(f"YOLO detect: {class_name}, dot matrix : {value}")
                subprocess.run(["/home/kjh/Modules/fpga_test_dot", str(value)])
            elif device_type == 'led':
                #print(f"YOLO detect: {class_name}, led : {value}")
                subprocess.run(["/home/kjh/Modules/fpga_test_led", str(value)])
            elif device_type == 'text_lcd':
                #print(f"YOLO detect: {class_name}, text lcd : {value}")
                subprocess.run(["/home/kjh/Modules/fpga_test_text_lcd", "hello", str(value)])
            elif device_type == 'fnd':
                #print(f"YOLO detect: {class_name}, fnd : {value}")
                subprocess.run(["/home/kjh/Modules/fpga_test_fnd", str(value)])
            elif device_type == 'buzzer':
                #print(f"YOLO detect: buzzer")
                subprocess.run(["/home/kjh/Modules/fpga_test_buzzer"])
        #else:
            #print(f"unknown: {class_name}")
    #else:
        #print("Not detecting")

    time.sleep(0.3)
