using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace KCS_GUI
{
	public partial class MainForm : Form
	{
		/* メンバ変数 */
		// 定数群
		//ソフト名
		const string SoftName = "KanColleSimulator";
		//艦隊数の最大(2)
		const int MaxFleetSize = 2;
		//種別→種別番号変換
		Dictionary<string, int> WeaponTypeToNumber;

		// 装備・艦娘データ
		//装備データ
		DataTable WeaponData;
		//装備ID→インデックス変換
		Dictionary<int, int> WeaponIDtoIndex;
		//種別番号→インデックスのリスト変換
		Dictionary<int, List<int>> WeaponTypeToIndexList;
		//熟練度が存在する装備の種別番号一覧
		List<int> RfWeaponTypeList;
		//艦娘データ
		DataTable KammusuData;
		//艦船ID→インデックス
		Dictionary<int, int> KammusuIDtoIndex;
		//艦種番号→インデックスのリスト変換
		Dictionary<int, List<int>> KammusuTypeToIndexList;
		// 画面表示用データ
		//艦隊タブ
		Fleet FormFleet;
		//マップタブ
		MapData FormMapData;

		/* コンストラクタ */
		public MainForm()
		{
			InitializeComponent();
			try
			{
				WeaponTypeToNumber = new Dictionary<string, int>() {
					{"主砲",0},
					{"対艦強化弾",1},
					{"副砲",2},
					{"魚雷",3},
					{"特殊潜航艇",4},
					{"艦上戦闘機",5},
					{"艦上爆撃機",6},
					{"艦上爆撃機(爆戦)",7},
					{"水上爆撃機",8},
					{"艦上攻撃機",9},
					{"艦上偵察機",10},
					{"艦上偵察機(彩雲)",11},
					{"大型飛行艇",12},
					{"水上偵察機",13},
					{"水上偵察機(夜偵)",14},
					{"対潜哨戒機",15},
					{"オートジャイロ",16},
					{"小型電探",17},
					{"大型電探",18},
					{"対空機銃",19},
					{"対空強化弾",20},
					{"高射装置",21},
					{"爆雷",22},
					{"ソナー",23},
					{"応急修理要員",24},
					{"探照灯",25},
					{"照明弾",26},
					{"艦隊司令部施設",27},
					{"水上艦要員",28},
					{"戦闘糧食",29},
					{"洋上補給",30},
					{"水上戦闘機",31},
					{"その他",32}
				};
				RfWeaponTypeList = new List<int> {
					WeaponTypeToNumber["艦上戦闘機"],
					WeaponTypeToNumber["艦上爆撃機"],
					WeaponTypeToNumber["艦上爆撃機(爆戦)"],
					WeaponTypeToNumber["水上爆撃機"],
					WeaponTypeToNumber["艦上攻撃機"],
					WeaponTypeToNumber["艦上偵察機"],
					WeaponTypeToNumber["艦上偵察機(彩雲)"],
					WeaponTypeToNumber["大型飛行艇"],
					WeaponTypeToNumber["水上戦闘機"]
				};
				ReadWeaponData();
				ReadKammusuData();
				RedrawWeaponNameList();
				RedrawKammusuNameList();
				RedrawMapKammusuNameList();
				FormFleet = new Fleet();
				FormMapData = new MapData();
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message, SoftName, MessageBoxButtons.OK, MessageBoxIcon.Error);
				this.Close();
			}
		}

		/* 各イベント毎の処理 */
		// メニュー
		private void NewFileMenuItem_Click(object sender, EventArgs e)
		{

		}
		private void OpenFileMenuItem_Click(object sender, EventArgs e)
		{

		}
		private void SaveSFileMenuItem_Click(object sender, EventArgs e)
		{

		}
		private void SaveAFileMenuItem_Click(object sender, EventArgs e)
		{

		}
		private void ExitMenuItem_Click(object sender, EventArgs e)
		{
			this.Close();
		}
		private void VersionInfoMenuItem_Click(object sender, EventArgs e)
		{

		}

		// 艦娘エディタタブ
		private void AddKammusuButton_Click(object sender, EventArgs e) {
			if(KammusuTypeComboBox.SelectedIndex == -1
			|| KammusuNameComboBox.SelectedIndex == -1
			|| FleetSelectComboBox.SelectedIndex == -1)
				return;
			DataRow[] dr = KammusuData.Select();
			// 艦娘データを作成する
			Kammusu setKammusu = new Kammusu();
			int index = KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex];
			setKammusu.id = int.Parse(dr[KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex][KammusuNameComboBox.SelectedIndex]]["艦船ID"].ToString());
			setKammusu.level = limit(int.Parse(KammusuLevelTextBox.Text), 1, 155);
			setKammusu.luck = limit(int.Parse(KammusuLuckTextBox.Text), 0, 100);
			setKammusu.cond = limit(int.Parse(KammusuCondTextBox.Text), 0, 100);
			FormFleet.unit[FleetSelectComboBox.SelectedIndex].Add(setKammusu);
			KammusuSelectListBox.Items.Add(dr[KammusuIDtoIndex[setKammusu.id]]["艦名"].ToString());
			KammusuSelectListBox.Refresh();
		}
		private void DeleteKammusuButton_Click(object sender, EventArgs e) {

		}
		private void AddWeaponButton_Click(object sender, EventArgs e) {

		}
		private void DeleteWeaponButton_Click(object sender, EventArgs e) {

		}
		private void KammusuTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawKammusuNameList();
		}
		private void WeaponTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawWeaponNameList();
		}

		// マップエディタタブ
		private void AddMapPositionButton_Click(object sender, EventArgs e) {

		}
		private void DeleteMapPositionButton_Click(object sender, EventArgs e) {

		}
		private void AddMapPatternButton_Click(object sender, EventArgs e) {

		}
		private void DeleteMapPatternButton_Click(object sender, EventArgs e) {

		}
		private void AddMapKammusuButton_Click(object sender, EventArgs e) {

		}
		private void DeleteMapKammusuButton_Click(object sender, EventArgs e) {

		}
		private void MapKammusuTypeComboBox_SelectedIndexChanged(object sender, EventArgs e) {
			RedrawMapKammusuNameList();
		}

		// シミュレーションタブ
		private void FriendBrowseButton_Click(object sender, EventArgs e)
		{

		}
		private void EnemyBrowseButton_Click(object sender, EventArgs e)
		{

		}
		private void OutputBrowseButton_Click(object sender, EventArgs e)
		{

		}
		private void StartButton_Click(object sender, EventArgs e)
		{

		}

		/* サブルーチン */
		// 装備データを読み込み
		private void ReadWeaponData()
		{
			// CSVをOLEで読み込む
			string csvDir = System.IO.Directory.GetCurrentDirectory();
			string csvFileName = "slotitems.csv";
			string conString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + csvDir + ";Extended Properties=\"text;HDR=Yes;FMT=Delimited\"";
			System.Data.OleDb.OleDbConnection con = new System.Data.OleDb.OleDbConnection(conString);
			string commText = "SELECT * FROM [" + csvFileName + "]";
			System.Data.OleDb.OleDbDataAdapter da = new System.Data.OleDb.OleDbDataAdapter(commText, con);
			WeaponData = new DataTable();
			da.Fill(WeaponData);
			// 読み込んだデータからインデックスを張る
			WeaponIDtoIndex = new Dictionary<int, int>();
			WeaponTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = WeaponData.Select();
			for (int i = 0; i < dr.Length; ++i){
				// ID→インデックスはそのまま
				WeaponIDtoIndex[int.Parse(dr[i]["装備ID"].ToString())] = i;
				// 種類→インデックスは例外を考慮する
				int type = WeaponTypeToNumber["その他"];
				if(WeaponTypeToNumber.ContainsKey(dr[i]["種別"].ToString())) {
					type = WeaponTypeToNumber[dr[i]["種別"].ToString()];
				}
				if(!WeaponTypeToIndexList.ContainsKey(type)) {
					WeaponTypeToIndexList.Add(type, new List<int>());
				}
				WeaponTypeToIndexList[type].Add(i);
			}
		}
		// 艦娘データを読み込み
		private void ReadKammusuData() {
			string csvDir = System.IO.Directory.GetCurrentDirectory();
			string csvFileName = "ships.csv";
			string conString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + csvDir + ";Extended Properties=\"text;HDR=Yes;FMT=Delimited\"";
			System.Data.OleDb.OleDbConnection con = new System.Data.OleDb.OleDbConnection(conString);
			string commText = "SELECT * FROM [" + csvFileName + "]";
			System.Data.OleDb.OleDbDataAdapter da = new System.Data.OleDb.OleDbDataAdapter(commText, con);
			KammusuData = new DataTable();
			da.Fill(KammusuData);
			// 読み込んだデータからインデックスを張る
			KammusuIDtoIndex = new Dictionary<int, int>();
			KammusuTypeToIndexList = new Dictionary<int, List<int>>();
			DataRow[] dr = KammusuData.Select();
			for(int i = 0; i < dr.Length; ++i) {
				// ID→インデックスはそのまま
				KammusuIDtoIndex[int.Parse(dr[i]["艦船ID"].ToString())] = i;
				// 種類→インデックスは例外を考慮する
				int type = int.Parse(dr[i]["艦種"].ToString()) - 1;	//1を引くのはインデックスとの対応のため
				if(!KammusuTypeToIndexList.ContainsKey(type)) {
					KammusuTypeToIndexList.Add(type, new List<int>());
				}
				KammusuTypeToIndexList[type].Add(i);
			}
		}
		// 装備データをGUIに反映
		private void RedrawWeaponNameList(){
			WeaponNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(WeaponTypeComboBox.SelectedIndex < 0) return;
			DataRow[] dr = WeaponData.Select();
			foreach(int index in WeaponTypeToIndexList[WeaponTypeComboBox.SelectedIndex]){
				WeaponNameComboBox.Items.Add(dr[index]["装備名"].ToString());
			}
			WeaponNameComboBox.Refresh();
			// 改修度および熟練度の選択を切り替える
			if(RfWeaponTypeList.IndexOf(WeaponTypeComboBox.SelectedIndex) != -1) {
				// 熟練度
				WeaponLevelComboBox.Enabled = false;
				WeaponRfComboBox.Enabled = true;
				WeaponDetailRfComboBox.Enabled = true;
			} else {
				// 改修度
				WeaponLevelComboBox.Enabled = true;
				WeaponRfComboBox.Enabled = false;
				WeaponDetailRfComboBox.Enabled = false;
			}
		}
		// 艦娘データをGUIに反映
		private void RedrawKammusuNameList() {
			KammusuNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(KammusuTypeComboBox.SelectedIndex < 0)
				return;
			DataRow[] dr = KammusuData.Select();
			foreach(int index in KammusuTypeToIndexList[KammusuTypeComboBox.SelectedIndex]) {
				KammusuNameComboBox.Items.Add(dr[index]["艦名"].ToString());
			}
			KammusuNameComboBox.Refresh();
		}
		private void RedrawMapKammusuNameList() {
			MapKammusuNameComboBox.Items.Clear();
			// 選択した種別に従って、リストを生成する
			if(MapKammusuTypeComboBox.SelectedIndex < 0)
				return;
			DataRow[] dr = KammusuData.Select();
			foreach(int index in KammusuTypeToIndexList[MapKammusuTypeComboBox.SelectedIndex]) {
				MapKammusuNameComboBox.Items.Add(dr[index]["艦名"].ToString());
			}
			MapKammusuNameComboBox.Refresh();
		}
		// 値を上下限で制限する
		private int limit(int n, int min_n, int max_n) {
			if(n < min_n)
				return min_n;
			if(n > max_n)
				return max_n;
			return n;
		}
		/* サブクラス */
		// 装備
		private class Weapon {
			// 装備ID
			public int id;
			// 装備改修度
			public int level;
			// 外部熟練度
			public int rf;
			// 内部熟練度
			public int detailRf;
		}
		// 艦娘
		private class Kammusu {
			// 艦船ID
			public int id;
			// レベル
			public int level;
			// 運
			public int luck;
			// cond値
			public int cond;
			// 装備
			public List<Weapon> weapon;
			// コンストラクタ
			public Kammusu() {
				weapon = new List<Weapon>();
			}
		}
		// 艦隊
		private class Fleet{
			// 司令部レベル
			public int level;
			// 艦隊形式
			public int type;
			// 艦娘
			public List<List<Kammusu>> unit;
			// コンストラクタ
			public Fleet() {
				unit = new List<List<Kammusu>>();
				for(int i = 0; i < MaxFleetSize; ++i) {
					unit.Add(new List<Kammusu>());
				}
			}
		}
		// マスデータ
		private class Position {
			// マスにおける戦闘モード
			public int mode;
			// マスの名称
			public string name;
			// 各パターンにおける陣形
			public List<int> formcation;
			// 各パターン
			public List<Fleet> fleet;
			// コンストラクタ
			public Position() {
				formcation = new List<int>();
				fleet = new List<Fleet>();
			}
		}
		// マップデータ
		private class MapData {
			// マス
			public List<Position> position;
			// コンストラクタ
			public MapData() {
				position = new List<Position>();
			}
		}
	}
}
