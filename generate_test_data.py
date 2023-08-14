import random
import csv

def generate_hex_string(length):
    """Generates a random hexadecimal string of the specified length."""
    return '0x' + ''.join(random.choices('0123456789abcdef', k=length)).zfill(12)

def generate_config_and_trace(num_rules, config_filename):
    """Generates configuration and trace files."""
    config_lines = ["[table_info]", f"project=lan_uc\nrule_num={num_rules}\n"]
    config_lines1 = [["key", "value"]]
    for i in range(num_rules):
        # key为48位，value为10位
        key = generate_hex_string(12)  # 48位需要12个十六进制字符

        value = generate_hex_string(3)  # 生成一个3位的十六进制字符串
        value = int(value, 16) & 0x3ff  # 将字符串转换为整数并应用位与操作
        value = '0x' + format(value, 'x').zfill(4)  # 将结果转换回字符串并填充前导零
        config_lines.append(f"[rule{i}]")
        config_lines.append(f"key={key}")
        config_lines.append(f"value={value}\n")
        config_lines1.append([key, value])  # 直接添加，不包含'0x'前缀
    # Save configuration to file
    with open(config_filename, 'w') as config_file:
        config_file.write("\n".join(config_lines))
    with open(f"{config_filename}.csv", 'w', newline='') as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["key", "value"])  # 添加CSV头部
        writer.writerows(config_lines1)

def main():
    # Get input from the user
    config_filename = input("Enter the filename to save the config: ")
    num_rules = int(input("Enter the number of rules: "))

    # Generate configuration and trace files
    generate_config_and_trace(num_rules, config_filename)

    print(f"Configuration saved to {config_filename}")

if __name__ == "__main__":
    main()
