using System;
using System.ComponentModel;
using System.Linq;
using Newtonsoft.Json;
using static KCS_GUI.CsvDataSet;

namespace KCS_GUI {
	// 装備
	[JsonObject(MemberSerialization = MemberSerialization.OptIn)]
	class Weapon {
		public readonly CsvDataSet.WeaponsRow row;
		readonly bool isAir;
		int level_;
		int rf_;
		int rf_detail_;

		// 装備ID
		[JsonProperty]
		public int id { get { return row.装備ID; } }

		public string 装備名 { get { return row.装備名; } }

		// rfのSerialzeに使用する
		[JsonProperty("rf"), EditorBrowsable(EditorBrowsableState.Never)]
		public int rf_or_level {
			get { return isAir ? rf : level; }
		}

		// 装備改修度
		public int level {
			get { return level_; }
			set { level_ = value.limit(0, 10); }
		}

		// 外部熟練度
		public int rf {
			get { return rf_; }
			set {
				rf_ = value.limit(0, 7);
				rf_detail_ = ToDetail(rf_);
			}
		}

		// 内部熟練度
		[JsonProperty]
		public int rf_detail {
			get { return rf_detail_; }
			set {
				rf_detail_ = value.limit(0, 120);
				rf_ = FromDetail(rf_detail_);
			}
		}

		public Weapon(int id) {
			// IDがデータベースに存在するか判定
			row = data.Weapons.SingleOrDefault(w => w.装備ID == id);
			if (row == null)
				throw new ArgumentOutOfRangeException("id");
			// 種別を判定することで、"rf"が装備改修度か艦載機熟練度かを判別する
			int setWeaponType;
			if (!WeaponTypeToNumber.TryGetValue(row.種別, out setWeaponType))
				setWeaponType = WeaponTypeToNumber["その他"];
			isAir = RfWeaponTypeList.Contains(setWeaponType);
		}

		public Weapon(int id, int level, int rf, int rf_detail) : this(id) {
			this.level = level;
			this.rf_ = rf.limit(0, 7);
			this.rf_detail_ = rf_detail.limit(0, 120);
		}

		[JsonConstructor]
		public Weapon(int id, int rf, int? rf_detail) : this(id) {
			if (isAir) {
				if (rf_detail == null)
					this.rf = rf;
				else
					this.rf_detail = rf_detail.Value;
			} else
				this.level = rf;
		}

		// JSONにrf_detailを書き出すか、Serializerが使用する
		[EditorBrowsable(EditorBrowsableState.Never)]
		public bool ShouldSerializerf_detail() {
			return isAir && ToDetail(rf) != rf_detail;
		}

		// 外部熟練度を内部熟練度に変換する
		public static int ToDetail(int rf) {
			return (rf * 15 - 5).limit(0, 120);
		}

		// 内部熟練度を外部熟練度に変換する
		public static int FromDetail(int rf_detail) {
			return ((rf_detail + 5) / 15).limit(0, 7);
		}
	}
}
