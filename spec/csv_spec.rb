require "spec_helper"
def check_csv_format(loaded_csv, regex)
  fail_count = 0
  loaded_csv.each_with_index{|line, i|
    next if i == 0
    unless line =~ regex
      puts line
      puts 'false'
      fail_count += 1
    end
  }
  if 0 != fail_count
    puts fail_count
  end
  return 0 == fail_count
end
describe "CSV test" do
  it "ships.csv" do
    expect(check_csv_format(
      File.read('ships.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[0-9]+,[0-9]+\/[0-9]+.[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+,[0-9],[0-9],[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+,[0-9]/
    )).to eq true
  end
  it "ships_all.csv" do
    expect(check_csv_format(
      File.read('ships_all.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[0-9]+,([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+|null),([0-9]+|null),([0-9]+|null),([0-9]+\/[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+|null),[0-9]/
    )).to eq true
  end
  it "slotitems.csv" do
    expect(check_csv_format(
      File.read('slotitems.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[^,]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+/
    )).to eq true
  end
end
