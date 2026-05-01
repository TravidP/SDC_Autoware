#!/bin/env python3

# This script is copied from this website https://y-t-g.github.io/tutorials/yolov8n-add-classes/
import os
import sys
from ament_index_python.packages import get_package_share_directory
package_share_directory = get_package_share_directory('custom_yolov8_model_generation')
ultralytics_path = os.path.join(package_share_directory, 'lib', 'ultralytics')
sys.path.insert(0, ultralytics_path)

import shutil
import torch
import copy
from ultralytics import YOLO
package_path = '/home/greendinokubra/yolov8_ws/src/object_detection_with_custom_model/custom_yolov8_model_generation'

# Train the dataset by using the coco yolov8 model
base_yolo_model = "yolov8m"
model = YOLO(f'{package_share_directory}/models/{base_yolo_model}.pt')
old_dict = copy.deepcopy(model.state_dict())
model.state_dict().keys()

def put_in_eval_mode(trainer, n_layers=22):
  for i, (name, module) in enumerate(trainer.model.named_modules()):
    if name.endswith("bn") and int(name.split('.')[1]) < n_layers:
      module.eval()
      module.track_running_stats = False
      # print(name, " put in eval mode.")

model.add_callback("on_train_epoch_start", put_in_eval_mode)
model.add_callback("on_pretrain_routine_start", put_in_eval_mode)

results = model.train(
    data=f'{package_path}/datasets/datasets/data.yaml', 
    freeze=22, 
    epochs=1, 
    imgsz=640, 
    batch=16)

# torch.save(old_dict, f'{package_share_directory}/new_folder/old_model.pth')
# torch.save(model.state_dict(), f'{package_share_directory}/new_folder/new_model.pth')

def compare_dicts(state_dict1, state_dict2):
    # Compare the keys
    keys1 = set(state_dict1.keys())
    keys2 = set(state_dict2.keys())

    if keys1 != keys2:
        print("Models have different parameter names.")
        return False

    # Compare the values (weights)
    for key in keys1:
        if not torch.equal(state_dict1[key], state_dict2[key]):
            print(f"Weights for parameter '{key}' are different.")
            if "bn" in key and "22" not in key:
              state_dict1[key] = state_dict2[key]

compare_dicts(old_dict, model.state_dict())

new_state_dict = dict()

#  Increment the head number by 1 in the state_dict
for k, v in model.state_dict().items():
  if k.startswith("model.model.22"):
    new_state_dict[k.replace("model.22", "model.23")] = v
  # else:
  #   new_state_dict[k] = v

number = 1
while True:
    new_dir = f'{package_path}/models/extending_{number}_{base_yolo_model}'
    if not os.path.exists(new_dir):
        os.makedirs(new_dir)
        break
    number += 1

torch.save(new_state_dict, f'{new_dir}/{base_yolo_model}_lp.pth')
shutil.copyfile(f'{ultralytics_path}/ultralytics/cfg/models/v8/{base_yolo_model}-2xhead.yaml', f'{new_dir}/{base_yolo_model}-2xhead.yaml')
shutil.copyfile(f'{package_share_directory}/models/{base_yolo_model}.pt', f'{new_dir}/{base_yolo_model}.pt')