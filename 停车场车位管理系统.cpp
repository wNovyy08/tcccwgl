#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <cstdio>

using namespace std;

// ---------- 基础定义 ----------
const int TOTAL_SPOTS = 50;
const double DEFAULT_RATE = 5.0;
const string DATA_FILE = "parking.dat";

// ---------- 枚举定义 ----------
enum ObjectType {
    OBJECT_UNKNOWN = 0,
    OBJECT_CAR = 1,
    OBJECT_TRUCK = 2,
    OBJECT_MOTORCYCLE = 3,
    OBJECT_PERSON = 4,
    OBJECT_BICYCLE = 5,
    OBJECT_OTHER = 99
};

enum GateStatus {
    GATE_CLOSED = 0,
    GATE_OPEN = 1
};

// 物体类型转字符串
string objectTypeToString(ObjectType type) {
    switch(type) {
        case OBJECT_CAR: return "小轿车";
        case OBJECT_TRUCK: return "卡车";
        case OBJECT_MOTORCYCLE: return "摩托车";
        case OBJECT_PERSON: return "行人";
        case OBJECT_BICYCLE: return "自行车";
        case OBJECT_UNKNOWN: return "未知物体";
        default: return "其他物体";
    }
}

// 判断是否为可入场车辆类型
bool isVehicleType(ObjectType type) {
    return (type == OBJECT_CAR || type == OBJECT_TRUCK || type == OBJECT_MOTORCYCLE);
}

// 获取当前时间字符串
string getCurrentTimeStr() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buf);
}

// 将 "YYYY-MM-DD HH:MM:SS" 转换为 time_t
time_t stringToTimeT(const string& timeStr) {
    tm tm_info = {};
    int year, month, day, hour, minute, second;
    sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = day;
    tm_info.tm_hour = hour;
    tm_info.tm_min = minute;
    tm_info.tm_sec = second;
    tm_info.tm_isdst = -1;
    return mktime(&tm_info);
}

// 计算两个时间字符串之间的秒数差
long long timeDiffSeconds(const string& start, const string& end) {
    time_t t_start = stringToTimeT(start);
    time_t t_end = stringToTimeT(end);
    return (long long)difftime(t_end, t_start);
}

// 将秒数转换为可读字符串
string secondsToHMS(long long seconds) {
    int hrs = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;
    ostringstream oss;
    oss << hrs << "小时" << mins << "分" << secs << "秒";
    return oss.str();
}

// ---------- 车辆信息 ----------
struct Vehicle {
    string licensePlate;
    string ownerName;
    string phone;
    string vehicleType;  // 车辆类型描述
    ObjectType objType;  // 物体识别类型
    
    Vehicle() : objType(OBJECT_CAR) {}
    Vehicle(const string& lp, const string& owner, const string& ph, const string& type, ObjectType ot = OBJECT_CAR)
        : licensePlate(lp), ownerName(owner), phone(ph), vehicleType(type), objType(ot) {}
    
    void display() const {
        cout << "车牌: " << licensePlate << ", 车主: " << ownerName
             << ", 电话: " << phone << ", 车型: " << vehicleType
             << ", 识别类型: " << objectTypeToString(objType);
    }
};

// ---------- 停车记录 ----------
struct ParkingRecord {
    string licensePlate;
    string ownerName;
    int spotId;
    string entryTime;
    string exitTime;
    double fee;
    bool isPaid;        // 是否已付费
    bool isCompleted;   // 是否已完成出场
    
    ParkingRecord() : spotId(0), fee(0.0), isPaid(false), isCompleted(false) {}
    
    void display() const {
        cout << "车牌: " << licensePlate << ", 车主: " << ownerName
             << ", 车位号: " << setw(3) << spotId;
        if (isCompleted) {
            cout << ", 入场: " << entryTime << ", 出场: " << exitTime
                 << ", 费用: " << fee << " 元"
                 << ", 付费: " << (isPaid ? "是" : "否");
        } else {
            cout << ", 入场: " << entryTime << ", 状态: 在场"
                 << ", 付费: " << (isPaid ? "是" : "否");
        }
        cout << endl;
    }
};

// ---------- 车位 ----------
struct ParkingSpot {
    int id;
    bool isOccupied;
    Vehicle currentVehicle;
    string entryTime;
    
    ParkingSpot(int _id) : id(_id), isOccupied(false) {}
    
    bool parkVehicle(const Vehicle& v, const string& timeStr) {
        if (isOccupied) return false;
        isOccupied = true;
        currentVehicle = v;
        entryTime = timeStr;
        return true;
    }
    
    string removeVehicle() {
        if (!isOccupied) return "";
        string retTime = entryTime;
        isOccupied = false;
        currentVehicle = Vehicle();
        entryTime.clear();
        return retTime;
    }
    
    void display() const {
        cout << "车位 " << setw(3) << id << " : ";
        if (isOccupied) {
            cout << "占用 [";
            currentVehicle.display();
            cout << ", 入场: " << entryTime << "]";
        } else {
            cout << "空闲";
        }
        cout << endl;
    }
};

// ---------- 摄像头模拟器 ----------
class Camera {
private:
    string location;  // 摄像头位置描述
    double accuracy;  // 识别准确率 (0.0-1.0)
    
public:
    Camera(const string& loc = "入口", double acc = 0.95) : location(loc), accuracy(acc) {}
    
    // 模拟识别物体，返回物体类型
    // 在实际应用中，这里会调用图像识别算法
    ObjectType detectObject() {
        // 模拟：90%概率识别为车辆，10%为其他物体
        int randVal = rand() % 100;
        
        if (randVal < 70) {
            return OBJECT_CAR;       // 70% 小轿车
        } else if (randVal < 80) {
            return OBJECT_TRUCK;     // 10% 卡车
        } else if (randVal < 85) {
            return OBJECT_MOTORCYCLE;// 5% 摩托车
        } else if (randVal < 90) {
            return OBJECT_PERSON;    // 5% 行人
        } else if (randVal < 95) {
            return OBJECT_BICYCLE;   // 5% 自行车
        } else {
            return OBJECT_UNKNOWN;   // 5% 未知
        }
    }
    
    // 手动指定识别结果（用于用户输入场景）
    ObjectType detectObjectManual(int choice) {
        switch(choice) {
            case 1: return OBJECT_CAR;
            case 2: return OBJECT_TRUCK;
            case 3: return OBJECT_MOTORCYCLE;
            case 4: return OBJECT_PERSON;
            case 5: return OBJECT_BICYCLE;
            default: return OBJECT_UNKNOWN;
        }
    }
    
    string getLocation() const { return location; }
};

// ---------- 栏杆控制器 ----------
class GateController {
private:
    string gateName;
    GateStatus status;
    
public:
    GateController(const string& name = "栏杆") : gateName(name), status(GATE_CLOSED) {}
    
    // 尝试抬起栏杆
    bool raise(const string& reason = "") {
        if (status == GATE_OPEN) {
            cout << "[" << gateName << "] 栏杆已经处于抬起状态。" << endl;
            return true;
        }
        status = GATE_OPEN;
        cout << ">>> [" << gateName << "] 栏杆抬起";
        if (!reason.empty()) cout << " (" << reason << ")";
        cout << " <<<" << endl;
        return true;
    }
    
    // 放下栏杆
    bool lower() {
        if (status == GATE_CLOSED) {
            return true;
        }
        status = GATE_CLOSED;
        cout << ">>> [" << gateName << "] 栏杆放下 <<<" << endl;
        return true;
    }
    
    GateStatus getStatus() const { return status; }
    bool isOpen() const { return status == GATE_OPEN; }
    string getStatusStr() const { return (status == GATE_OPEN) ? "抬起" : "关闭"; }
    string getName() const { return gateName; }
};

// ---------- 停车场管理系统 ----------
class ParkingLot {
private:
    vector<ParkingSpot> spots;
    vector<ParkingRecord> records;
    double hourlyRate;
    int totalSpots;
    
    // 摄像头和栏杆
    Camera entryCamera;
    Camera exitCamera;
    GateController entryGate;
    GateController exitGate;
    
    int findVehicleSpot(const string& licensePlate) {
        for (size_t i = 0; i < spots.size(); ++i) {
            if (spots[i].isOccupied && spots[i].currentVehicle.licensePlate == licensePlate) {
                return i;
            }
        }
        return -1;
    }
    
    int findFreeSpot() {
        for (size_t i = 0; i < spots.size(); ++i) {
            if (!spots[i].isOccupied) return i;
        }
        return -1;
    }
    
    double calculateFee(long long seconds) const {
        if (seconds <= 0) return 0.0;
        int hours = seconds / 3600;
        if (seconds % 3600 != 0) hours++;
        return hours * hourlyRate;
    }
    
    int findActiveRecord(const string& licensePlate) {
        for (size_t i = 0; i < records.size(); ++i) {
            if (!records[i].isCompleted && records[i].licensePlate == licensePlate) {
                return i;
            }
        }
        return -1;
    }
    
public:
    ParkingLot(int total = TOTAL_SPOTS, double rate = DEFAULT_RATE) 
        : totalSpots(total), hourlyRate(rate),
          entryCamera("入口摄像头", 0.95),
          exitCamera("出口摄像头", 0.95),
          entryGate("入口栏杆"),
          exitGate("出口栏杆") {
        for (int i = 1; i <= totalSpots; ++i) {
            spots.push_back(ParkingSpot(i));
        }
        srand((unsigned int)time(0));  // 初始化随机种子
    }
    
    // ========== 摄像头识别入口车辆 ==========
    bool processEntryDetection() {
        cout << "\n========== 入口摄像头检测 ==========" << endl;
        cout << "[" << entryCamera.getLocation() << "] 正在检测物体..." << endl;
        
        // 模拟摄像头识别
        ObjectType detected = entryCamera.detectObject();
        cout << "检测结果: " << objectTypeToString(detected) << endl;
        
        if (!isVehicleType(detected)) {
            cout << ">>> 非车辆物体 (" << objectTypeToString(detected) 
                 << ")，入口栏杆不抬起 <<<" << endl;
            return false;
        }
        
        cout << ">>> 识别为车辆类型 (" << objectTypeToString(detected) 
             << ")，允许通行 <<<" << endl;
        entryGate.raise("车辆检测通过");
        return true;
    }
    
    // 手动指定识别结果（用户交互用）
    bool processEntryDetectionManual(int detectChoice) {
        cout << "\n========== 入口摄像头检测 ==========" << endl;
        ObjectType detected = entryCamera.detectObjectManual(detectChoice);
        cout << "手动指定检测结果: " << objectTypeToString(detected) << endl;
        
        if (!isVehicleType(detected)) {
            cout << ">>> 非车辆物体 (" << objectTypeToString(detected) 
                 << ")，入口栏杆不抬起 <<<" << endl;
            return false;
        }
        
        cout << ">>> 识别为车辆类型 (" << objectTypeToString(detected) 
             << ")，允许通行 <<<" << endl;
        entryGate.raise("车辆检测通过");
        return true;
    }
    
    // ========== 车辆入场 ==========
    bool vehicleEntry(const Vehicle& v) {
        if (findVehicleSpot(v.licensePlate) != -1) {
            cout << "错误：车牌 " << v.licensePlate << " 已在停车场内！" << endl;
            entryGate.lower();
            return false;
        }
        
        if (!entryGate.isOpen()) {
            cout << "错误：入口栏杆未抬起，车辆无法入场！" << endl;
            return false;
        }
        
        int idx = findFreeSpot();
        if (idx == -1) {
            cout << "停车场已满，无空闲车位！" << endl;
            entryGate.lower();
            return false;
        }
        
        string now = getCurrentTimeStr();
        spots[idx].parkVehicle(v, now);
        
        ParkingRecord rec;
        rec.licensePlate = v.licensePlate;
        rec.ownerName = v.ownerName;
        rec.spotId = spots[idx].id;
        rec.entryTime = now;
        rec.isPaid = false;
        rec.isCompleted = false;
        records.push_back(rec);
        
        cout << ">>> 车辆 " << v.licensePlate << " 已停入 " << spots[idx].id 
             << " 号车位，入场时间: " << now << " <<<" << endl;
        
        // 车辆通过后放下栏杆
        entryGate.lower();
        return true;
    }
    
    // ========== 缴费处理 ==========
    bool processPayment(const string& licensePlate) {
        int idx = findVehicleSpot(licensePlate);
        if (idx == -1) {
            cout << "错误：未找到车牌 " << licensePlate << " 的车辆！" << endl;
            return false;
        }
        
        string entryTime = spots[idx].entryTime;
        string currentTime = getCurrentTimeStr();
        long long seconds = timeDiffSeconds(entryTime, currentTime);
        double fee = calculateFee(seconds);
        
        cout << "\n========== 缴费处理 ==========" << endl;
        cout << "车牌: " << licensePlate << endl;
        cout << "入场时间: " << entryTime << endl;
        cout << "当前时间: " << currentTime << endl;
        cout << "停留时长: " << secondsToHMS(seconds) << endl;
        cout << "应缴费用: " << fee << " 元" << endl;
        
        int recIdx = findActiveRecord(licensePlate);
        if (recIdx != -1) {
            records[recIdx].fee = fee;
        }
        
        cout << "确认缴费？(Y/N): ";
        char confirm;
        cin >> confirm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (confirm == 'Y' || confirm == 'y') {
            if (recIdx != -1) {
                records[recIdx].isPaid = true;
            }
            cout << ">>> 缴费成功！费用: " << fee << " 元 <<<" << endl;
            return true;
        } else {
            cout << "缴费已取消。" << endl;
            return false;
        }
    }
    
    // ========== 出口检测与出场 ==========
    bool processExitDetection(const string& licensePlate) {
        cout << "\n========== 出口摄像头检测 ==========" << endl;
        
        int spotIdx = findVehicleSpot(licensePlate);
        if (spotIdx == -1) {
            cout << "错误：车辆 " << licensePlate << " 不在停车场内！" << endl;
            return false;
        }
        
        // 检查是否已缴费
        int recIdx = findActiveRecord(licensePlate);
        if (recIdx == -1 || !records[recIdx].isPaid) {
            cout << ">>> 车辆 " << licensePlate << " 尚未缴费！" << endl;
            cout << ">>> 出口栏杆不抬起，请先完成缴费！ <<<" << endl;
            return false;
        }
        
        // 模拟出口摄像头再次确认是车辆
        cout << "[" << exitCamera.getLocation() << "] 正在检测物体..." << endl;
        ObjectType detected = exitCamera.detectObject();
        cout << "检测结果: " << objectTypeToString(detected) << endl;
        
        if (!isVehicleType(detected)) {
            cout << ">>> 警告：检测到非车辆物体，请人工确认！ <<<" << endl;
        }
        
        // 缴费成功且识别通过，抬起出口栏杆
        exitGate.raise("缴费验证通过");
        return true;
    }
    
    // 车辆出场
    bool vehicleExit(const string& licensePlate) {
        if (!exitGate.isOpen()) {
            cout << "错误：出口栏杆未抬起！请先通过出口检测并缴费。" << endl;
            return false;
        }
        
        int idx = findVehicleSpot(licensePlate);
        if (idx == -1) {
            cout << "错误：未找到车牌 " << licensePlate << " 的车辆！" << endl;
            exitGate.lower();
            return false;
        }
        
        string entryTime = spots[idx].removeVehicle();
        string exitTime = getCurrentTimeStr();
        long long seconds = timeDiffSeconds(entryTime, exitTime);
        double fee = calculateFee(seconds);
        
        cout << ">>> 车辆 " << licensePlate << " 出场，停留 " << secondsToHMS(seconds) 
             << "，费用: " << fee << " 元 <<<" << endl;
        
        int recIdx = findActiveRecord(licensePlate);
        if (recIdx != -1) {
            records[recIdx].exitTime = exitTime;
            records[recIdx].fee = fee;
            records[recIdx].isCompleted = true;
        }
        
        // 车辆通过后放下栏杆
        exitGate.lower();
        return true;
    }
    
    // ========== 状态显示 ==========
    void showGateStatus() const {
        cout << "\n========== 栏杆状态 ==========" << endl;
        cout << "入口栏杆: " << entryGate.getStatusStr() 
             << " (" << entryCamera.getLocation() << ")" << endl;
        cout << "出口栏杆: " << exitGate.getStatusStr() 
             << " (" << exitCamera.getLocation() << ")" << endl;
    }
    
    void showAllSpots() const {
        cout << "\n========== 车位状态 (共 " << spots.size() << " 个) ==========" << endl;
        for (size_t i = 0; i < spots.size(); ++i) {
            spots[i].display();
        }
        int occupied = 0;
        for (size_t i = 0; i < spots.size(); ++i) {
            if (spots[i].isOccupied) occupied++;
        }
        cout << "当前占用: " << occupied << ", 空闲: " << spots.size() - occupied << endl;
    }
    
    void queryVehicle(const string& licensePlate) const {
        int idx = -1;
        for (size_t i = 0; i < spots.size(); ++i) {
            if (spots[i].isOccupied && spots[i].currentVehicle.licensePlate == licensePlate) {
                idx = i;
                break;
            }
        }
        if (idx == -1) {
            cout << "车辆 " << licensePlate << " 不在停车场内。" << endl;
            for (int i = records.size() - 1; i >= 0; --i) {
                if (records[i].licensePlate == licensePlate) {
                    cout << "最近一次记录：" << endl;
                    records[i].display();
                    return;
                }
            }
            cout << "无相关记录。" << endl;
        } else {
            cout << "车辆在场：";
            spots[idx].display();
            // 显示付费状态
            for (size_t i = 0; i < records.size(); ++i) {
                if (!records[i].isCompleted && records[i].licensePlate == licensePlate) {
                    cout << "付费状态: " << (records[i].isPaid ? "已付费" : "未付费") << endl;
                    break;
                }
            }
        }
    }
    
    void showCurrentVehicles() const {
        cout << "\n========== 在场车辆 ==========" << endl;
        bool found = false;
        for (size_t i = 0; i < spots.size(); ++i) {
            if (spots[i].isOccupied) {
                spots[i].display();
                // 显示付费状态
                for (size_t j = 0; j < records.size(); ++j) {
                    if (!records[j].isCompleted && 
                        records[j].licensePlate == spots[i].currentVehicle.licensePlate) {
                        cout << "    -> 付费状态: " << (records[j].isPaid ? "已付费" : "未付费") << endl;
                        break;
                    }
                }
                found = true;
            }
        }
        if (!found) cout << "当前无车辆。" << endl;
    }
    
    void showHistory() const {
        cout << "\n========== 停车历史记录 (共 " << records.size() << " 条) ==========" << endl;
        if (records.empty()) {
            cout << "暂无记录。" << endl;
            return;
        }
        for (size_t i = 0; i < records.size(); ++i) {
            records[i].display();
        }
    }
    
    void showStatistics() const {
        int activeVehicles = 0;
        for (size_t i = 0; i < spots.size(); ++i) {
            if (spots[i].isOccupied) activeVehicles++;
        }
        double totalRevenue = 0.0;
        int completed = 0;
        int unpaid = 0;
        for (size_t i = 0; i < records.size(); ++i) {
            if (records[i].isCompleted) {
                totalRevenue += records[i].fee;
                completed++;
            } else if (!records[i].isPaid) {
                unpaid++;
            }
        }
        cout << "\n========== 统计信息 ==========" << endl;
        cout << "总车位数: " << spots.size() << endl;
        cout << "当前在场车辆: " << activeVehicles << endl;
        cout << "在场未付费车辆: " << unpaid << endl;
        cout << "历史记录总数: " << records.size() << " (已完成: " << completed << ")" << endl;
        cout << "累计收入: " << totalRevenue << " 元" << endl;
        if (activeVehicles > 0) {
            cout << "当前占用率: " << (activeVehicles * 100.0 / spots.size()) << "%" << endl;
        }
    }
    
    // 数据持久化
    bool saveToFile() {
        ofstream fout(DATA_FILE.c_str(), ios::binary);
        if (!fout) {
            cerr << "无法保存数据到文件 " << DATA_FILE << endl;
            return false;
        }
        
        fout.write((const char*)&totalSpots, sizeof(totalSpots));
        fout.write((const char*)&hourlyRate, sizeof(hourlyRate));
        
        int spotCount = spots.size();
        fout.write((const char*)&spotCount, sizeof(spotCount));
        for (size_t i = 0; i < spots.size(); ++i) {
            const ParkingSpot& sp = spots[i];
            fout.write((const char*)&sp.id, sizeof(sp.id));
            fout.write((const char*)&sp.isOccupied, sizeof(sp.isOccupied));
            if (sp.isOccupied) {
                size_t len = sp.currentVehicle.licensePlate.size();
                fout.write((const char*)&len, sizeof(len));
                fout.write(sp.currentVehicle.licensePlate.c_str(), len);
                
                len = sp.currentVehicle.ownerName.size();
                fout.write((const char*)&len, sizeof(len));
                fout.write(sp.currentVehicle.ownerName.c_str(), len);
                
                len = sp.currentVehicle.phone.size();
                fout.write((const char*)&len, sizeof(len));
                fout.write(sp.currentVehicle.phone.c_str(), len);
                
                len = sp.currentVehicle.vehicleType.size();
                fout.write((const char*)&len, sizeof(len));
                fout.write(sp.currentVehicle.vehicleType.c_str(), len);
                
                int ot = (int)sp.currentVehicle.objType;
                fout.write((const char*)&ot, sizeof(ot));
                
                len = sp.entryTime.size();
                fout.write((const char*)&len, sizeof(len));
                fout.write(sp.entryTime.c_str(), len);
            }
        }
        
        int recCount = records.size();
        fout.write((const char*)&recCount, sizeof(recCount));
        for (size_t i = 0; i < records.size(); ++i) {
            const ParkingRecord& rec = records[i];
            size_t len = rec.licensePlate.size();
            fout.write((const char*)&len, sizeof(len));
            fout.write(rec.licensePlate.c_str(), len);
            
            len = rec.ownerName.size();
            fout.write((const char*)&len, sizeof(len));
            fout.write(rec.ownerName.c_str(), len);
            
            fout.write((const char*)&rec.spotId, sizeof(rec.spotId));
            
            len = rec.entryTime.size();
            fout.write((const char*)&len, sizeof(len));
            fout.write(rec.entryTime.c_str(), len);
            
            len = rec.exitTime.size();
            fout.write((const char*)&len, sizeof(len));
            fout.write(rec.exitTime.c_str(), len);
            
            fout.write((const char*)&rec.fee, sizeof(rec.fee));
            fout.write((const char*)&rec.isPaid, sizeof(rec.isPaid));
            fout.write((const char*)&rec.isCompleted, sizeof(rec.isCompleted));
        }
        fout.close();
        return true;
    }
    
    bool loadFromFile() {
        ifstream fin(DATA_FILE.c_str(), ios::binary);
        if (!fin) return false;
        
        fin.read((char*)&totalSpots, sizeof(totalSpots));
        fin.read((char*)&hourlyRate, sizeof(hourlyRate));
        
        int spotCount;
        fin.read((char*)&spotCount, sizeof(spotCount));
        spots.clear();
        for (int i = 0; i < spotCount; ++i) {
            int id;
            bool occupied;
            fin.read((char*)&id, sizeof(id));
            fin.read((char*)&occupied, sizeof(occupied));
            ParkingSpot spot(id);
            spot.isOccupied = occupied;
            if (occupied) {
                size_t len;
                string temp;
                
                fin.read((char*)&len, sizeof(len));
                temp.resize(len);
                fin.read(&temp[0], len);
                spot.currentVehicle.licensePlate = temp;
                
                fin.read((char*)&len, sizeof(len));
                temp.resize(len);
                fin.read(&temp[0], len);
                spot.currentVehicle.ownerName = temp;
                
                fin.read((char*)&len, sizeof(len));
                temp.resize(len);
                fin.read(&temp[0], len);
                spot.currentVehicle.phone = temp;
                
                fin.read((char*)&len, sizeof(len));
                temp.resize(len);
                fin.read(&temp[0], len);
                spot.currentVehicle.vehicleType = temp;
                
                int ot;
                fin.read((char*)&ot, sizeof(ot));
                spot.currentVehicle.objType = (ObjectType)ot;
                
                fin.read((char*)&len, sizeof(len));
                temp.resize(len);
                fin.read(&temp[0], len);
                spot.entryTime = temp;
            }
            spots.push_back(spot);
        }
        
        int recCount;
        fin.read((char*)&recCount, sizeof(recCount));
        records.clear();
        for (int i = 0; i < recCount; ++i) {
            ParkingRecord rec;
            size_t len;
            string temp;
            
            fin.read((char*)&len, sizeof(len));
            temp.resize(len);
            fin.read(&temp[0], len);
            rec.licensePlate = temp;
            
            fin.read((char*)&len, sizeof(len));
            temp.resize(len);
            fin.read(&temp[0], len);
            rec.ownerName = temp;
            
            fin.read((char*)&rec.spotId, sizeof(rec.spotId));
            
            fin.read((char*)&len, sizeof(len));
            temp.resize(len);
            fin.read(&temp[0], len);
            rec.entryTime = temp;
            
            fin.read((char*)&len, sizeof(len));
            temp.resize(len);
            fin.read(&temp[0], len);
            rec.exitTime = temp;
            
            fin.read((char*)&rec.fee, sizeof(rec.fee));
            fin.read((char*)&rec.isPaid, sizeof(rec.isPaid));
            fin.read((char*)&rec.isCompleted, sizeof(rec.isCompleted));
            
            records.push_back(rec);
        }
        fin.close();
        return true;
    }
    
    double getRate() const { return hourlyRate; }
    
    void setRate(double newRate) {
        if (newRate > 0) {
            hourlyRate = newRate;
            cout << "费率已更新为: " << hourlyRate << " 元/小时" << endl;
        } else {
            cout << "费率必须大于0！" << endl;
        }
    }
};

// ---------- 辅助输入函数 ----------
string readLine(const string& prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

double readDouble(const string& prompt) {
    cout << prompt;
    double val = 0;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请重新输入数字: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return val;
}

int readInt(const string& prompt) {
    cout << prompt;
    int val = 0;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请重新输入数字: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return val;
}

// ---------- 菜单 ----------
void showMainMenu() {
    cout << "\n========== 停车场管理系统 (带摄像头+栏杆) ==========" << endl;
    cout << "1. 车辆入场流程 (摄像头检测→栏杆抬起→停车)" << endl;
    cout << "2. 缴费 (输入车牌缴费)" << endl;
    cout << "3. 车辆出场流程 (出口检测→栏杆抬起→出场)" << endl;
    cout << "4. 查询车辆信息" << endl;
    cout << "5. 显示在场车辆" << endl;
    cout << "6. 显示所有车位状态" << endl;
    cout << "7. 显示栏杆状态" << endl;
    cout << "8. 查看历史记录" << endl;
    cout << "9. 停车统计" << endl;
    cout << "10. 设置费率" << endl;
    cout << "11. 保存数据到文件" << endl;
    cout << "0. 退出系统" << endl;
    cout << "请选择: ";
}

// ---------- 主函数 ----------
int main() {
    ParkingLot lot;
    
    if (lot.loadFromFile()) {
        cout << "已加载停车场数据。" << endl;
    } else {
        cout << "首次运行，初始化停车场 (车位数: " << TOTAL_SPOTS << ")。" << endl;
    }
    
    int choice;
    while (true) {
        showMainMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "无效输入，请重新选择。" << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (choice == 1) {
            // 完整入场流程
            cout << "\n===== 车辆入场流程 =====" << endl;
            
            // 步骤1：摄像头检测
            cout << "选择检测模式：" << endl;
            cout << "  1 - 自动检测（模拟摄像头识别）" << endl;
            cout << "  2 - 手动指定物体类型" << endl;
            int mode = readInt("请选择: ");
            
            bool detected = false;
            if (mode == 2) {
                cout << "\n手动指定物体类型：" << endl;
                cout << "  1 - 小轿车" << endl;
                cout << "  2 - 卡车" << endl;
                cout << "  3 - 摩托车" << endl;
                cout << "  4 - 行人" << endl;
                cout << "  5 - 自行车" << endl;
                int typeChoice = readInt("请选择: ");
                detected = lot.processEntryDetectionManual(typeChoice);
            } else {
                detected = lot.processEntryDetection();
            }
            
            if (!detected) {
                cout << ">>> 入场流程终止：非车辆物体 <<<" << endl;
                continue;
            }
            
            // 步骤2：输入车辆信息并停车
            string lp = readLine("\n请输入车牌号: ");
            if (lp.empty()) {
                cout << "车牌号不能为空！入场流程终止。" << endl;
                continue;
            }
            string owner = readLine("请输入车主姓名: ");
            string phone = readLine("请输入联系电话: ");
            string type = readLine("请输入车辆类型描述: ");
            Vehicle v(lp, owner, phone, type, OBJECT_CAR);
            lot.vehicleEntry(v);
        }
        else if (choice == 2) {
            // 缴费
            string lp = readLine("请输入缴费车牌号: ");
            if (lp.empty()) {
                cout << "车牌号不能为空！" << endl;
                continue;
            }
            lot.processPayment(lp);
        }
        else if (choice == 3) {
            // 完整出场流程
            cout << "\n===== 车辆出场流程 =====" << endl;
            string lp = readLine("请输入出场车牌号: ");
            if (lp.empty()) {
                cout << "车牌号不能为空！" << endl;
                continue;
            }
            
            // 步骤1：出口检测（验证付费状态）
            if (lot.processExitDetection(lp)) {
                // 步骤2：车辆出场
                lot.vehicleExit(lp);
            } else {
                cout << ">>> 出场流程终止：未通过出口验证 <<<" << endl;
            }
        }
        else if (choice == 4) {
            string lp = readLine("请输入查询车牌号: ");
            lot.queryVehicle(lp);
        }
        else if (choice == 5) {
            lot.showCurrentVehicles();
        }
        else if (choice == 6) {
            lot.showAllSpots();
        }
        else if (choice == 7) {
            lot.showGateStatus();
        }
        else if (choice == 8) {
            lot.showHistory();
        }
        else if (choice == 9) {
            lot.showStatistics();
        }
        else if (choice == 10) {
            double rate = readDouble("请输入新费率 (元/小时): ");
            lot.setRate(rate);
        }
        else if (choice == 11) {
            if (lot.saveToFile()) {
                cout << "数据保存成功！" << endl;
            } else {
                cout << "数据保存失败！" << endl;
            }
        }
        else if (choice == 0) {
            cout << "是否保存数据后退出？(Y/N): ";
            char ans;
            cin >> ans;
            if (ans == 'Y' || ans == 'y') {
                lot.saveToFile();
            }
            cout << "感谢使用停车场管理系统，再见！" << endl;
            break;
        }
        else {
            cout << "无效选项，请重新选择。" << endl;
        }
    }
    return 0;
}
