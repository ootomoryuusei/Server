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

// エントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetWindowText("タイトル");

	int NetUDPHandle;
	int RecvSize, TotalRecvSize;

	SetGraphMode(800,600, 32);
	ChangeWindowMode(TRUE); // Windowモードの場合

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	NetUDPHandle = MakeUDPSocket(SERVER_PORT); //UDP通信用のソケットハンドルの作成

	DrawString(0, 0, "受信待ち", GetColor(255, 255, 255));

	while (CheckNetWorkRecvUDP(NetUDPHandle) == FALSE) {
		if (ProcessMessage() < 0) break;
	}
	
	//受信
	NetWorkRecvUDP(NetUDPHandle, NULL, NULL, Buff, 256, FALSE);

	std::string line = Buff;
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

	/*SetRankings(Buff, 1724);
	SortScore();*/




	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);
	/*ClearDrawScreen();*/
	DrawString(0, 0, Buff, GetColor(255, 255, 255));

	DeleteUDPSocket(NetUDPHandle);

	

	//TotalRecvSize = 0;
	//while (ProcessMessage() == 0) {


	//	
	//	ClearDrawScreen();

	//	//データ受信
	//	RecvSize = NetWorkRecvUDP(NetUDPHandle, NULL, NULL, Data, sizeof(Data), FALSE);
	//	if (RecvSize >= 0) {
	//		TotalRecvSize += RecvSize;
	//	}
	//	DrawFormatString(0, 0, GetColor(255, 255, 255), "TotalRecvSize:%d", TotalRecvSize);

	//	ScreenFlip();

	//	if (DxLib::ProcessMessage() == -1 || CheckHitKey(KEY_INPUT_ESCAPE) == 1)
	//		break;
	//}
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

	std::ofstream ofs_csv_file(output_csv_file_path_SortData);
	ofs_csv_file << "PlayerName" << "," << "PlayerScore";
	ofs_csv_file << std::endl;
	for (auto itr = r.begin(); itr != r.end(); ++itr) {
		ofs_csv_file << itr->second << "," << itr->first;
		ofs_csv_file << std::endl;
		ofs_csv_file.close();
	}
}
