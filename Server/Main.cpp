/// <summary>
/// DxLib を入れたバージョン
///  2023/12/6 花井
/// </summary>

#include"DxLib.h"
#include <string>
#include<fstream>
#include"Engine/CsvReader.h"
#include<map>
#include<algorithm>
#include<sstream>


namespace {
	BYTE Data[500];
	char Buff[256];
	const unsigned short SERVER_PORT = 8888;
	const unsigned short CLIENT_PORT = 8080;
	std::vector<std::string> rank;
	std::vector<std::string> name;
	std::vector<std::string> score;
	std::string SendData;

	IPDATA IpAddr;
	
	CsvReader* csv;
	int height, width;
	std::map <std::string, float> Rankings;
	std::vector<std::string> RankingData;
	std::vector<std::pair<float, std::string>> r;
	std::string output_csv_file_path_ScoreData = "Assets\\Rankings\\RankingsSystem.csv";
	std::string output_csv_file_path_SortData = "Assets\\Rankings\\RankingsSystemClearSort.csv";

	std::vector<int> RecvData;
}

void SetRankings(std::string _Pname, float _Pscore);
void SortScore();
void SarchMyRank(std::string myName);

// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetWindowText("タイトル");
	SetGraphMode(800, 600, 32);
	ChangeWindowMode(TRUE);

	int Ycount = 0;
	int NetUDPHandle;
	int RecvSize, TotalRecvSize;

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);
	SetAlwaysRunFlag(1);

	while (ProcessMessage() == 0) {
		//SetGraphMode(800, 600, 32);
		//ChangeWindowMode(TRUE); // Windowモードの場合

		//if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
		//{
		//	return -1;			// エラーが起きたら直ちに終了
		//}
		ClearDrawScreen();
	
		NetUDPHandle = MakeUDPSocket(SERVER_PORT); //UDP通信用のソケットハンドルの作成

		DrawString(0, 20*Ycount, "受信待ち", GetColor(255, 255, 255));
		Ycount++;

		while (CheckNetWorkRecvUDP(NetUDPHandle) == FALSE) {
			if (ProcessMessage() < 0) break;
		}

		//受信
		NetWorkRecvUDP(NetUDPHandle, NULL, NULL, Buff, 256, FALSE);

		std::string RecvBuff = Buff;
		std::stringstream RLine(RecvBuff);
		std::string c;
		std::vector<std::string> RvBuff;


		while (std::getline(RLine, c, ':')) {
			RvBuff.push_back(c);
		}

		std::stringstream NandS(RvBuff[0]);
		std::string N;
		float S;
		int count = 0;

		while (std::getline(NandS, c, '.')) {
			if (count == 0) {
				N = c;
				
			}
			if (count == 1) {
				S = atof(c.c_str());
			}
			count++;
		}

		SetRankings(N,S);
		SortScore();

		DrawString(0, 20 * Ycount, "受信しました", GetColor(255, 255, 255));
		Ycount++;
		DrawString(0, 20 * Ycount,RvBuff[0].c_str(), GetColor(255, 255, 255));
		Ycount++;

		IPDATA IPA;
		GetNetWorkIP(NetUDPHandle, &IPA);

		DeleteUDPSocket(NetUDPHandle);


		

		std::string line = RvBuff[1];
		std::stringstream line2(line);
		std::string s;
		unsigned char ip[4];

		while (std::getline(line2, s, '.')) {
			RecvData.push_back(atoi(s.c_str()));
		}

		for (auto itr = RecvData.begin(); itr != RecvData.end(); itr++) {
			size_t index = std::distance(RecvData.begin(), itr);
			ip[index] = *itr;
		}

		//UDP通信用ソケットハンドルを作成
		NetUDPHandle = MakeUDPSocket(-1);
		IpAddr.d1 = ip[0];
		IpAddr.d2 = ip[1];
		IpAddr.d3 = ip[2];
		IpAddr.d4 = ip[3];

		/*csv = new CsvReader(output_csv_file_path_SortData);

		height = csv->GetLines();
		if (height > 1) {
			for (int x = 0; x < csv->GetColumns(0); x++) {
				for (int y = 1; y < csv->GetLines(); y++) {
					RankingData.push_back(csv->GetString(y, x));
					if (!(x == csv->GetColumns(0) - 1 && y == csv->GetLines() - 1) && !(x == 0 && y == csv->GetLines() - 1)) {
						RankingData.push_back(".");
					}
					if (x == 0 && y == csv->GetLines() - 1) {
						RankingData.push_back(":");
					}
				}
			}
		}

		std::string rData;

		for (auto itr : RankingData) {
			rData += itr;
		}*/
		SarchMyRank(N);
		// 文字列の送信
		NetWorkSendUDP(NetUDPHandle, IpAddr, CLIENT_PORT,SendData.c_str(), SendData.size());

		DrawString(0, 20*Ycount,SendData.c_str(), GetColor(255, 255, 255));
		Ycount++;

		/*SetBackgroundColor(0, 0, 0);
		SetDrawScreen(DX_SCREEN_BACK);*/
		/*ClearDrawScreen();*/
		

		DeleteUDPSocket(NetUDPHandle);
		RankingData.clear();
		Rankings.clear();
		r.clear();
		name.clear();
		rank.clear();
		score.clear();
		ScreenFlip();
	}
	DxLib_End();				// ＤＸライブラリ使用の終了処理
	return 0;
}

void SetRankings(std::string _Pname, float _Pscore)
{
	std::ofstream ofs_csv_file(output_csv_file_path_ScoreData, std::ios::app);
	ofs_csv_file << _Pname << "," << _Pscore;
	ofs_csv_file << std::endl;
	ofs_csv_file.close();
}

void SortScore()
{
	csv = new CsvReader(output_csv_file_path_ScoreData);
	height = csv->GetLines();
	if (height > 1) {
		for (int h = 1; h < height; h++) {
			Rankings.insert(std::pair<std::string, float>(csv->GetString(h, 0), csv->GetFloat(h, 1)));
		}

		for (std::map<std::string, float>::iterator it = Rankings.begin(); it != Rankings.end(); it++) {
			r.push_back({ it->second, it->first });
		}

		sort(r.rbegin(), r.rend());
	}

	std::ofstream ofs_csv_file(output_csv_file_path_SortData, std::ios::out);
	ofs_csv_file << "PlayerName" << "," << "PlayerScore";
	ofs_csv_file << std::endl;
	for (auto itr = r.begin(); itr != r.end(); ++itr) {
		ofs_csv_file << itr->second << "," << itr->first;
		ofs_csv_file << std::endl;
	}
	ofs_csv_file.close();
}

void SarchMyRank(std::string myName) {
	int myRank = 0;
	int rTop = 1;
	int rLow = 0;
	int myHei = 0;
	int myLow = 0;

	csv = new CsvReader(output_csv_file_path_SortData);
	for (int h = 1; h < csv->GetLines(); h++) {
		if (myName == csv->GetString(h, 0)) {
			myRank = h;
		}
	}
	rLow = csv->GetLines() - 1;

	myHei = myRank - rTop;
	int fRank = myHei - 3;
	if (fRank < 1) {
		fRank = 1;
	}
	int length = fRank + 5;
	if (length > csv->GetLines() - 1) {
		int over = length - (csv->GetLines());
		length = length - over;
	}
	for (int i = fRank; i < length; i++) {
		name.push_back(csv->GetString(i, 0));
		score.push_back(csv->GetString(i, 1));
		rank.push_back(std::to_string(i));
	}

	std::string nData;
	std::string sData;
	std::string rData;

	for (auto itr = name.begin(); itr != name.end(); itr++) {
		nData += *itr;
		if (!(itr == name.end() - 1)) {
			nData += ".";
		}
	}

	for (auto itr = score.begin(); itr != score.end(); itr++) {
		sData += *itr;
		if (!(itr == score.end() - 1)) {
			sData += ".";
		}
	}

	for (auto itr = rank.begin(); itr != rank.end(); itr++) {
		rData += *itr;
		if (!(itr == rank.end() - 1)) {
			rData += ".";
		}
	}

	SendData = rData + ":" + nData + ":" + sData;
}
