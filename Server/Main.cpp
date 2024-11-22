/// <summary>
/// DxLib ����ꂽ�o�[�W����
///  2023/12/6 �Ԉ�
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

// �G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetWindowText("�^�C�g��");

	int Ycount = 0;
	int NetUDPHandle;
	int RecvSize, TotalRecvSize;

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

		DrawString(0, 20 * Ycount, "��M���܂���", GetColor(255, 255, 255));
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

		//UDP�ʐM�p�\�P�b�g�n���h�����쐬
		NetUDPHandle = MakeUDPSocket(-1);
		IpAddr.d1 = ip[0];
		IpAddr.d2 = ip[1];
		IpAddr.d3 = ip[2];
		IpAddr.d4 = ip[3];

		csv = new CsvReader(output_csv_file_path_SortData);

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
		}
		// ������̑��M
		NetWorkSendUDP(NetUDPHandle, IpAddr, CLIENT_PORT,rData.c_str(),rData.size());

		DrawString(0, 20*Ycount,rData.c_str(), GetColor(255, 255, 255));
		Ycount++;

		/*SetBackgroundColor(0, 0, 0);
		SetDrawScreen(DX_SCREEN_BACK);*/
		/*ClearDrawScreen();*/
		

		DeleteUDPSocket(NetUDPHandle);
		RankingData.clear();
		Rankings.clear();
		r.clear();
	}
		DxLib_End();				// �c�w���C�u�����g�p�̏I������
	
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

void MakeSendData(std::string myName) {
	int myRank = 0;
	int myHei = 0;
	int myLow = 0;
	csv = new CsvReader(output_csv_file_path_SortData);

	for (int h = 1; h < csv->GetLines(); h++) {
		if (myName == csv->GetString(h, 0)) {
			myRank = h;
		}
	}

	myHei = myRank - 2;
	myLow = (csv->GetLines() - 1) - myRank;




}
