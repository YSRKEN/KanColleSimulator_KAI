using System.Collections.Generic;

namespace KCS_GUI {
	partial class CsvDataSet {
		// 装備・艦娘データ
		internal static readonly CsvDataSet data = new CsvDataSet();

		internal static readonly string[] shipTypes = {
			"魚雷艇",
			"駆逐艦",
			"軽巡洋艦",
			"重雷装巡洋艦",
			"重巡洋艦",
			"航空巡洋艦",
			"軽空母",
			"巡洋戦艦",
			"戦艦",
			"航空戦艦",
			"正規空母",
			"陸上型",
			"潜水艦",
			"潜水空母",
			"輸送艦",
			"水上機母艦",
			"揚陸艦",
			"装甲空母",
			"工作艦",
			"潜水母艦",
			"練習巡洋艦",
			"給油艦",
		};

		//種別→種別番号変換
		internal static readonly Dictionary<string, int> WeaponTypeToNumber = new Dictionary<string, int> {
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
		//熟練度が存在する装備の種別番号一覧
		internal static readonly HashSet<int> RfWeaponTypeList = new HashSet<int> {
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
		static CsvDataSet() {
			using (var adapter = new CsvDataSetTableAdapters.ShipsTableAdapter())
				adapter.Fill(data.Ships);
			using (var adapter = new CsvDataSetTableAdapters.WeaponsTableAdapter())
				adapter.Fill(data.Weapons);
		}
	}
}
