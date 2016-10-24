require "spec_helper"
require 'kconv'
def check_csv_format(loaded_csv, regex)
  fail_count = 0
  loaded_csv.each_with_index{|line, i|
    next if i == 0
    unless line =~ regex
      if 0 == fail_count
        puts 'fail line :'
      end
      puts line.toutf8
      fail_count += 1
    end
  }
  return fail_count
end
describe "CSV test" do
  it "ships.csv" do
    expect(check_csv_format(
      File.read('ships.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[0-9]+,[0-9]+\/[0-9]+.[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+,[0-9],[0-9],[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[0-9]+\/[0-9]+,[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+,[0-9]/
    )).to eq 0
  end
  it "ships_all.csv" do
    expect(check_csv_format(
      File.read('ships_all.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[0-9]+,([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+|null),([0-9]+|null),([0-9]+|null),([0-9]+\/[0-9]+\/[0-9]+\/[0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([0-9]+\/[0-9]+|null),([-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+\/[-0-9]+|null),[0-9]/
    )).to eq 0
  end
  it "slotitems.csv" do
    expect(check_csv_format(
      File.read('slotitems.csv', encoding:'Shift_JIS').split("\n"), 
      /[0-9]+,[^,]+,[^,]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+,[-0-9]+/
    )).to eq 0
  end
end
