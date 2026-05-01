import os

import rclpy
from rclpy.node import Node

from ultralytics import YOLO

from ament_index_python.packages import get_package_share_directory


class YOLOTrainNode(Node):
    def __init__(self):
        super().__init__('yolo_train_node')

        # Params
        self.declare_parameter("model", "yolov8n.pt")
        model = self.get_parameter(
            "model").get_parameter_value().string_value

        self.declare_parameter("yaml_file", "custom_model.yaml")
        yaml_file = self.get_parameter(
            "yaml_file").get_parameter_value().string_value

        self.declare_parameter("image_size", 1280)
        image_size = self.get_parameter(
            "image_size").get_parameter_value().integer_value

        self.declare_parameter("epochs", 50)
        epochs = self.get_parameter(
            "epochs").get_parameter_value().integer_value

        self.declare_parameter("batch_size", 8)
        batch_size = self.get_parameter(
            "batch_size").get_parameter_value().integer_value

        self.declare_parameter("model_name", "yolov8n_v8_1e")
        model_name = self.get_parameter(
            "model_name").get_parameter_value().string_value

        # Load the model.
        self.model = YOLO(model)

        # Training parameters
        self.data_path = os.path.join(
            get_package_share_directory('custom_yolov8_model_generation'),
            'config',
            yaml_file
        )
        self.img_size = image_size
        self.epochs = epochs
        self.batch_size = batch_size
        self.model_name = model_name

        # Call the training function
        self.train_model()

    def train_model(self):
        # Training.
        results = self.model.train(
            data=self.data_path,
            imgsz=self.img_size,
            epochs=self.epochs,
            batch=self.batch_size,
            name=self.model_name
        )

def main(args=None):
    rclpy.init(args=args)
    node = YOLOTrainNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
