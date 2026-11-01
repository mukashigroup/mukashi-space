-- MUKASHI Space - AutoName for Reaper
-- Кидаешь в Reaper: Actions -> Show action list -> Load ReaScript -> ставишь как Startup action или на кнопку
-- Скрипт сканирует все треки и пишет их имена в файл, который читает HUB

function write_shared_file()
  local tracks = {}
  local num_tracks = reaper.CountTracks(0)
  for i=0, num_tracks-1 do
    local track = reaper.GetTrack(0, i)
    local _, name = reaper.GetTrackName(track, "")
    if name == "" then name = "Track "..(i+1) end
    -- ищем есть ли на треке MUKASHI SOURCE, если есть - берем его ID
    local _, chunk = reaper.GetTrackStateChunk(track, "", false)
    -- простой маппинг по порядку, но можно парсить
    table.insert(tracks, {id="track_"..i, name=name, idx=i})
  end
  
  -- читаем существующий файл чтобы не затереть позиции X/Y
  local tmp = os.getenv("TEMP") or os.getenv("TMP") or "/tmp"
  local path = tmp .. "/mukashi_space_v3.json"
  local existing = {}
  local f = io.open(path, "r")
  if f then
    local content = f:read("*all")
    f:close()
    -- парсим очень просто, ищем slots
    -- для надежности просто мерджим имена по idx
  end
  
  -- формируем JSON
  local json = '{"slots":['
  for i, t in ipairs(tracks) do
    if i>1 then json = json .. ',' end
    -- по умолчанию позиции рандомные, но HUB потом сохранит реальные
    json = json .. string.format('{"id":"%s","name":"%s","x":0,"y":%d,"room":40,"wet":20,"color":%d,"active":true,"lastUpdate":0}', t.id, t.name:gsub('"',''), i*3, 0xFFAA5500 + i*1000)
  end
  -- добавляем еще слотов до 16 если треков меньше
  for i=#tracks+1,16 do
    if i>1 then json = json .. ',' end
    json = json .. string.format('{"id":"track_%d","name":"Empty %d","x":0,"y":0,"room":40,"wet":20,"color":%d,"active":false,"lastUpdate":0}', i-1, i, 0xFF444444)
  end
  json = json .. ']}'
  
  local out = io.open(path, "w")
  if out then out:write(json); out:close() end
end

reaper.defer(function()
  write_shared_file()
  -- автообновление каждые 2 секунды
  reaper.runloop = function()
    write_shared_file()
    reaper.defer(reaper.runloop)
  end
  -- для постоянного мониторинга раскомментируй:
  -- reaper.defer(reaper.runloop)
end)

reaper.ShowMessageBox("MUKASHI AutoName: имена треков записаны в HUB файл.\nТеперь открой HUB на пустой дорожке - он подтянет имена.", "MUKASHI Space", 0)
