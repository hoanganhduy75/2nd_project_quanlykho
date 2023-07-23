//
//  main.cpp
//  Project
//
//  Created by Hoang Anh Duy on 17/03/2023.
//
#include "json.hpp"
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <iomanip>

#include <zbar.h>
using namespace cv;
using namespace zbar;

using namespace std;
using json = nlohmann::json;

struct phone {
  string barcode;
  string brand;
  string model;
  int quantity;
};
struct deletePhone {
  string barcode;
  string brand;
  string model;
  int quantity;
    char *date;
    string reason;
};
struct decodedObject
{
  string type; //Type of barcode, usually CODE-128
  string data; //Data of barcode
  vector <Point> location;
} ;
vector<decodedObject> decodedObjects;
vector<struct phone> phones_list;
vector<struct deletePhone> delete_phones_list;
time_t now = time(0);

//Generate random string for barcode
string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string tmp_s;
    tmp_s.reserve(len);
    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return tmp_s;
}

//Print menu
int menu() {
  initscr(); // initial screen
  raw();
  cbreak();  // set input mode
  noecho();  // enable echo mode for screen
  keypad(stdscr, true);
  string choices[] = {"1. Nhap kho ", "2. Kiem tra kho", "3. Xuat kho",
                      "4. Exit"};
  int selected = 0;
  while (true) {
    clear();
    printw("CHUONG TRINH BUON BAN DA CAP\n");
    for (int i = 0; i < 4; i++) {
      if (i == selected) {
        printw("> ");
      } else {
        printw("  ");
      }
      printw("%s\n", choices[i].c_str());
    }
    int input = getch();
    switch (input) {
    case KEY_UP:
      selected = (selected - 1 + 4) % 4;
      break;
    case KEY_DOWN:
      selected = (selected + 1) % 4;
      break;
    case '\n':
      clear();
      printw("You selected %s.\n", choices[selected].c_str());
      refresh(); // refresh windows
      endwin();
      return selected;
    default:
      break;
    }
    refresh();
  }
}

//Open Json file to read
void readJsonFile(fstream &data, json js) {
    for (auto &obj : js) {
        struct phone phone;
        if (!(obj["barcode"].is_null())) {
            phone.barcode = obj["barcode"];
        }
        if (!(obj["brand"].is_null())) {
            phone.brand = obj["brand"];
        }
        if (!(obj["model"].is_null())) {
            phone.model = obj["model"];
        }
        if (!(obj["quantity"].is_null())) {
            phone.quantity = obj["quantity"];
        }
        phones_list.push_back(phone);
    }
}

//Write js to data.json
void writeJsonFile(fstream &data, json js)
{
    if (data.is_open()) {
        ofstream data("/Users/yudhna_/Desktop/Project/Project_OK/data.json", ios::trunc);
        data << std::setw(4) << std::setfill(' ') << js;
        data.close();
    }
    else {
        cout << "bug mo file de viet: " << endl;
    }
}

//Print phones list
void printPhoneList() {
  for (auto &obj : phones_list) {
    cout << "Barcode: " << obj.barcode << endl;
    cout << "Brand: " << obj.brand << endl;
    cout << "Model: " << obj.model << endl;
    cout << "Quantity: " << obj.quantity << endl;
    cout << endl;
  }
}

//Print phone in4
void printPhone(auto &obj) {
  cout << "Barcode: " << obj.barcode << endl;
  cout << "Brand : " << obj.brand << endl;
  cout << "Model: " << obj.model << endl;
  cout << "Quantity: " << obj.quantity << endl;
  cout << endl;
}

//Fil phone with string value
void filProduct(string key, string value, json js) {
  system("clear");
  int count = 0;
    if (key == "barcode") {
      count = 0;
      for (auto &obj : phones_list) {
        if (obj.barcode == value) {
          printPhone(obj);
          count++;
        }
      }
      if (count == 0)
        cout << "Khong co san pham trung khop" << endl;
    }
    if (key == "brand") {
      count = 0;
      for (auto &obj : phones_list) {
        if (obj.brand == value) {
          printPhone(obj);
          count++;
        }
      }
      if (count == 0)
        cout << "Khong co san pham trung khop" << endl;
    }
  if (key == "model") {
    count = 0;
    for (auto &obj : phones_list) {
      if (obj.model == value) {
        printPhone(obj);
        count++;
      }
    }
    if (count == 0)
      cout << "Khong co san pham trung khop" << endl;
  }
}

//Fil phone with integer value
void filProduct(string key, int value, json js) {
  system("clear");
  int count = 0;
  if (key == "quantity") {
    count = 0;
    for (auto &obj : phones_list) {
      if (obj.quantity == value) {
        printPhone(obj);
        count++;
      }
    }
    if (count == 0)
      cout << "Khong co san pham trung khop" << endl;
  }
}

//Checking string is number ?
bool is_number(const std::string &s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && isdigit(*it))
    ++it;
  return !s.empty() && it == s.end();
}
//Delete phone from Phoneslist
void delete_Phone(int countEle)
{
    phones_list.erase(phones_list.begin() + countEle - 1);
}
// Find and decode barcodes and QR codes
bool decode(Mat &im, vector<decodedObject>&decodedObjects)
{
  ImageScanner scanner;
  scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
  Mat imGray;
    cvtColor(im, imGray,COLOR_BGR2GRAY);
  Image image(im.cols, im.rows, "Y800", (uchar *)imGray.data, im.cols * im.rows);
  int n = scanner.scan(image);
  for(Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
  {
    decodedObject obj;
    obj.type = symbol->get_type_name();
    obj.data = symbol->get_data();
    decodedObjects.push_back(obj);
      return true;
  }
    return false;
}
//Feature1
bool feature1(fstream &data, json &js, int count)
{
    system("clear");
    if (data.is_open())
    {
            refresh();
            string choices[] = {"Use Barcode", "Keyboard"};
            int selected = 0;
            while (true) {
                clear();
                printw("Ban muon nhap theo: \n");
                int numEle = sizeof(choices)/sizeof(choices[0]);
                for (int i = 0; i < numEle; i++) {
                    if (i == selected) {
                        printw("> ");
                    } else {
                        printw("  ");
                    }
                    printw("%s\n", choices[i].c_str());
                }
                int input = getch();
                switch (input) {
                    case KEY_UP:
                        selected = (selected - 1 + numEle) % numEle;
                        break;
                    case KEY_DOWN:
                        selected = (selected + 1) % numEle;
                        break;
                    case '\n':
                        if (count != 0)
                            cin.ignore();
                        clear();
                        refresh(); // refresh windows
                        endwin();
                    default:
                        break;
                }
                if (input == '\n')
                    break;
            }
        vector<struct phone> input_phones_list;
        if (selected == 0)
        {
            Mat image, bbox, image2;
            namedWindow("Scan_Window",WINDOW_NORMAL);
            VideoCapture cap(0);
            cap = VideoCapture("http://yudhna.local:8081/video");
            if (!cap.isOpened()) {
                cout << "cannot open camera";
            }
            vector<decodedObject> decodedObjects;
            while (true) {
                cap >> image;
                cv::flip(image, image2, 1);
                resizeWindow("Scan_Window", 620 , 350);
                imshow("Scan_Window", image2);
                if (decode(image2, decodedObjects) ==  true)
                {
                    cv::destroyWindow("Scan_Window");
                    cv::waitKey(1);
                    cap.release();
                    break;
                }
                waitKey(25);
            }
            struct phone phone;
            string temp = "";
            temp = decodedObjects[0].data;
            int countPipe = 0;
            
            char * cstr = new char [temp.length()+1];
            strcpy (cstr, temp.c_str());
            char * p = strtok (cstr,"|");
            while (p!=0)
            {
                if (countPipe == 0)
                    phone.barcode = p;
                if (countPipe == 1)
                    phone.brand = p;
                if (countPipe == 2)
                    phone.model = p;
                if (countPipe == 3)
                    phone.quantity = stoi(p);
                countPipe++;
                p = std::strtok(NULL,"|");
            }
            input_phones_list.push_back(phone);
            delete[] cstr;
        }
        if (selected == 1)
        {
            int numBrand = 0;
            cout << "Nhap so dong san pham: ";
            cin >> numBrand;
            system("clear");
            string barcode = "";
            barcode = gen_random(6);
            for(int i = 0; i < numBrand; i++)
            {
                struct phone phone;
                phone.barcode = barcode;
                cout << "Nhap ten dong san pham thu " << i+1 << " : ";
                cin.ignore();
                getline(cin, phone.brand);
                cout << "Nhap model thu " << i+1 << " : ";
                //            cin.ignore();
                getline(cin,phone.model);
                cout << "Nhap so luong thu " << i+1 << " : ";
                cin >> phone.quantity;
                input_phones_list.push_back(phone);
            }
        }
        
        for(auto &obj : input_phones_list)
        {
            /*/js.pushback({something}) is different from js.pushback({something2}, {something3})
               this will put something1 to new array    this will put something2 and something3 to the same array which included something0 */
            js.push_back({
                {"barcode", obj.barcode},
                {"brand", obj.brand},
                {"model", obj.model},
                {"quantity", obj.quantity}
            });
        }
        writeJsonFile(data, js);
    }
    else
    {
        cout << "Loi mo file feature 1";
    }
    char cont;
    system("clear");
    cout << "Ban co muon tiep tuc ? (Y/N): ";
    cin >> cont;
    if (cont == 'Y')
        return true;
    else
        return false;
}

//Feature 2: Find product
bool feature2(json js, int count) {
    string key;
    refresh();
    string choices[] = {"Barcode", "Brand", "Model", "Quantity"};
    int selected = 0;
    while (true) {
        clear();
        printw("Ban muon tim kiem theo: \n");
        int numEle = sizeof(choices)/sizeof(choices[0]);
        for (int i = 0; i < numEle; i++) {
            if (i == selected) {
                printw("> ");
            } else {
                printw("  ");
            }
            printw("%s\n", choices[i].c_str());
        }
        int input = getch();
        switch (input) {
            case KEY_UP:
                selected = (selected - 1 + numEle) % numEle;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % numEle;
                break;
            case '\n':
                if (count != 0)
                    cin.ignore();
                clear();
                key = choices[selected];
                transform(key.begin(), key.end(), key.begin(), ::tolower);
//                refresh(); // refresh windows
                endwin();
            default:
                break;
        }
        if (input == '\n')
            break;
    }
    string value;
    cout << "Vui long nhap " << choices[selected] << " de tim kiem: ";
    getline(cin, value);
    if (is_number(value) && key == "quantity") {
        filProduct(key, stoi(value), js);
    } else {
        if (key == "barcode" || key == "brand" || key == "model") {
            filProduct(key, value, js);
        } else {
            cout << "Vui long nhap dung kieu gia tri" << endl;
        }
    }
    char cont;
    cout << "Ban co muon tiep tuc ? (Y/N): ";
    cin >> cont;
    if (cont == 'Y')
        return true;
    else
        return false;
}
//Feature3
bool feature3(fstream &data, json &js, int count)
{
    refresh();
    string choices[] = {"Ban san pham", "Xoa san pham"};
    int selected = 0;
    while (true) {
        clear();
        printw("Chon phuong thuc dieu chinh: \n");
        int numEle = sizeof(choices)/sizeof(choices[0]);
        for (int i = 0; i < numEle; i++) {
            if (i == selected) {
                printw("> ");
            } else {
                printw("  ");
            }
            printw("%s\n", choices[i].c_str());
        }
        int input = getch();
        switch (input) {
            case KEY_UP:
                selected = (selected - 1 + numEle) % numEle;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % numEle;
                break;
            case '\n':
                if (count != 0)
                    cin.ignore();
                clear();
                refresh(); // refresh windows
                endwin();
            default:
                break;
        }
        if (input == '\n')
            break;
    }
    system("clear");
again1:
    string barcode = "";
    cout << "Nhap barcode san pham ban muon dieu chinh: ";
    getline(cin,barcode);
    int countP = 0;
    for(auto &obj : phones_list)
    {
        if (barcode == obj.barcode)
        {
            countP++;
            printPhone(obj);
        }
    }
again:
    string delete_model = "";
    int delete_quantity = 0;
    if (countP != 0)
    {
        cout << "Tim thay " << countP << " san pham." << endl;
        cout << "Vui long nhap model: ";
        getline(cin,delete_model);
        cout << "Vui long nhap so luong: ";
        cin >> delete_quantity;
    }
    else
    {
        cout << "Khong tim thay san pham !" << endl;
        goto again1;
    }
    int countEle = 0;
    for(auto &obj : phones_list)
    {
        countEle++;
        struct deletePhone phone;
        if (barcode == obj.barcode && delete_model == obj.model)
        {
            phone.barcode = obj.barcode;
            phone.brand = obj.brand;
            phone.model = obj.model;
            phone.date = ctime(&now);
            if (choices[selected] == "Ban san pham")
            {
                phone.reason = "Sold";
            }
            if (choices[selected] == "Xoa san pham")
            {
                phone.reason = "Deleted";
            }
            if (obj.quantity - delete_quantity > 0)
            {
                js.at(countEle-1)["quantity"] = obj.quantity - delete_quantity;
                obj.quantity -= delete_quantity;
                phone.quantity = delete_quantity;
                writeJsonFile(data, js);
            }
            else {
                if (obj.quantity - delete_quantity < 0)
                {
                    cout << "So luong khong du vui long thao tac lai !" << endl;
                    goto again;
                }
                else {
                    if (obj.quantity - delete_quantity == 0)
                    {
                        phone.quantity = delete_quantity;
                        delete_Phone(countEle);
                        js.erase(js.begin()+ countEle -1);
                        writeJsonFile(data, js);
                    }
                }
            }
        }
        if (barcode == obj.barcode && delete_model == obj.model)
        {
            delete_phones_list.push_back(phone);
        }
    }
    fstream deletef("/Users/yudhna_/Desktop/Project/Project_OK/data.json");
    json delete_js = json::parse(deletef);
    for(auto &obj : delete_phones_list)
    {
        delete_js.push_back({
            {"barcode", obj.barcode},
            {"brand", obj.brand},
            {"model", obj.model},
            {"quantity", obj.quantity},
            {"date", obj.date},
            {"reason", obj.reason}
        });
    }
    if (deletef.is_open()) {
        ofstream deletef("/Users/yudhna_/Desktop/Project/Project_OK/delete.json", ios::trunc);
        deletef << std::setw(4) << std::setfill(' ') << delete_js;
        deletef.close();
        char cont;
        cout << "Ban co muon tiep tuc ? (Y/N): ";
        cin >> cont;
        if (cont == 'Y')
            return true;
        else
            return false;
    }
    else
    {
        cout << "Loi mo file feature 3";
    }
    return false;
}
/*/---------Main------------*/
int main() {
    begin:
    delete_phones_list.clear();
    phones_list.clear();
    int count = 0;
    srand((unsigned)time(NULL) * getpid());
    system("clear");
    fstream data("/Users/yudhna_/Desktop/Project/Project_OK/data.json");
    json js = json::parse(data);
    if (data.is_open())
    {
        readJsonFile(data, js);
        int selected = menu();
        count++;
        switch (selected)
        {
            case 0:
                while(feature1(data,js,count))
                {
                }
                data.close();
                goto begin;
                break;
            case 1:
//                count = 0;
                while (feature2(js,count)) {
                }
                data.close();
                goto begin;
                break;
            case 2:
//                count = 0;
                while(feature3(data, js,count))
                {
//                    count1 ++;
                }
                data.close();
                goto begin;
                break;
            case 3:
                cout << "Xin tam biet" << endl;
                data.close();
                break;
            default:
                cout << "Co bugggg" << endl;
                break;
        }
    }
    else
        cout << "Chuong trinh co loi";
}
