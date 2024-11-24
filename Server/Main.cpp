/// <summary>
/// DxLib ����ꂽ�o�[�W����
///  2023/12/6 �Ԉ�
/// </summary>

#include <string>
#include<fstream>
#include"Engine/CsvReader.h"
#include<map>
#include<algorithm>
#include<sstream>
#include "DxLib.h"


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
	std::string output_csv_file_path_ScoreData = "Assets\\Rankings\\RankingsSystem";
	std::string output_csv_file_path_SortData = "Assets\\Rankings\\RankingsSystemClearSort";

	std::vector<int> RecvData;
}

void SetRankings(std::string _mapnum,std::string _Pname, float _Pscore);
void SortScore(std::string _mapnum);
void SarchMyRank(std::string _mapnum, std::string myName);

// �G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetWindowText("�^�C�g��");

	int Ycount = 0;
	int NetUDPHandle;
	int RecvSize, TotalRecvSize;
	SetAlwaysRunFlag(TRUE);

	while (ProcessMessage() == 0) {
		SetGraphMode(800, 600, 32);
		ChangeWindowMode(TRUE); // Window���[�h�̏ꍇ

		if (DxLib_Init() == -1)		// �c�w���C�u��������������
		{
			return -1;			// �G���[���N�����璼���ɏI��
		}
	
		NetUDPHandle = MakeUDPSocket(SERVER_PORT); //UDP�ʐM�p�̃\�P�b�g�n���h���̍쐬

		DrawString(0, 20*Ycount, "��M�҂�", GetColor(255, 255, 255));
		Ycount++;

		while (CheckNetWorkRecvUDP(NetUDPHandle) == FALSE) {
			if (ProcessMessage() < 0) break;
		}

		//��M
		NetWorkRecvUDP(NetUDPHandle, NULL, NULL, Buff, 256, FALSE);

		std::string RecvBuff = Buff;
		std::stringstream RLine(RecvBuff);
		std::string c;
		std::vector<std::string> RvBuff;

		//���O�X�R�A��IP�𕪗�
		while (std::getline(RLine, c, ':')) {
			RvBuff.push_back(c);
		}

		std::stringstream NandS(RvBuff[0]);
		std::string N;
		float S = -1;
		int count = 0;
		std::string mapnum;

		//���O��IP�𕪗�
		while (std::getline(NandS, c, '.')) {
			if (count == 0) {
				mapnum = c;
			}
			if (count == 1) {
				N = c;
				
			}
			if (count == 2) {
				S = atof(c.c_str());
			}
			count++;
		}

		//�����L���O�ɑ}��
		SetRankings(mapnum, N, S);
		
		SortScore(mapnum);

		DrawString(0, 20 * Ycount, "��M���܂���", GetColor(255, 255, 255));
		Ycount++;
		DrawString(0, 20 * Ycount,RvBuff[0].c_str(), GetColor(255, 255, 255));
		Ycount++;

		IPDATA IPA;
		GetNetWorkIP(NetUDPHandle, &IPA);

		DeleteUDPSocket(NetUDPHandle);


		
		//IP�̕����E�\�z
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

		//UDP�ʐM�p�\�P�b�g�n���h�����쐬
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
		SarchMyRank(mapnum,N);
		// ������̑��M
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
	}
		DxLib_End();				// �c�w���C�u�����g�p�̏I������
	
	return 0;
}

void SetRankings(std::string _mapnum , std::string _Pname, float _Pscore)
{
	std::ofstream ofs_csv_file(output_csv_file_path_ScoreData + _mapnum + ".csv", std::ios::app);
	ofs_csv_file << _Pname << "," << _Pscore;
	ofs_csv_file << std::endl;
	ofs_csv_file.close();
}

void SortScore(std::string _mapnum)
{
	csv = new CsvReader(output_csv_file_path_ScoreData + _mapnum + ".csv");
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

	if (csv != nullptr) {
		delete csv;
		csv = nullptr;
	}

	std::ofstream ofs_csv_file(output_csv_file_path_SortData + _mapnum + ".csv", std::ios::out);
	ofs_csv_file << "PlayerName" << "," << "PlayerScore";
	ofs_csv_file << std::endl;
	for (auto itr = r.begin(); itr != r.end(); ++itr) {
		ofs_csv_file << itr->second << "," << itr->first;
		ofs_csv_file << std::endl;
	}
	ofs_csv_file.close();
}

void SarchMyRank(std::string _mapnum,std::string myName) {
	int myRank = 0;
	int rTop = 1;
	int rLow = 0;
	int myHei = 0;
	int myLow = 0;

	csv = new CsvReader(output_csv_file_path_SortData + _mapnum + ".csv");
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

	if (csv != nullptr) {
		delete csv;
		csv = nullptr;
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
