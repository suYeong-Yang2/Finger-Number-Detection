#!/bin/bash
cd /home/kjh/Modules

echo "🧹 Removing existing modules..."
rmmod fpga_dot_driver 2>/dev/null
rmmod fpga_interface_driver 2>/dev/null

echo "🔌 Inserting fpga_interface_driver.ko..."
insmod fpga_interface_driver.ko || { echo "❌ Failed to insert fpga_interface_driver.ko"; exit 1; }

#echo "🔌 Inserting fpga_dot_driver.ko..."
#insmod fpga_dot_driver.ko || { echo "❌ Failed to insert fpga_dot_driver.ko"; exit 1; }

echo "✅ Modules loaded."

