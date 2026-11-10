-- Paste into the Lua console of examples/repl or the gallery's Developer tools
-- page. widget(f) hands the function to whichever slot the app calls, so the
-- demo above the console is replaced wherever it lives. widget(nil) restores
-- the app's own card.
local say = note or toast or function() end        -- note() in the gallery, toast() in repl

local function graph(ui, self)
  self.items  = self.items or {}
  self.next   = self.next or 0
  self.draft  = self.draft or ""
  self.filter = self.filter or "all"

  local function add()
    if self.draft ~= "" then
      self.next = self.next + 1
      table.insert(self.items, { id = self.next, text = self.draft, done = false, star = 0 })
      local t = self.draft
      self.draft = ""
      say("added: " .. t)
      ui.toast("task added", { tone = "success", description = t, duration = 2 })
    end
  end

  ui.card({ gap = 12, width = "fill", pad = 14 }, function()
    ui.row({ gap = 8, align = "center" }, function()
      ui.icon("list", 18)
      ui.heading("tasks", 3)
      ui.spacer()
      local open = 0
      for _, t in ipairs(self.items) do if not t.done then open = open + 1 end end
      ui.badge(open .. " open", { tone = open > 0 and "info" or "success", dot = true })
    end)

    self.draft, self.sent = ui.textfield("draft", self.draft, {
      placeholder = "what needs doing?", icon = "plus", clearable = true })
    if self.sent then add() end

    ui.row({ gap = 8 }, function()
      if ui.button("Add", { variant = "primary", icon = "plus" }) then add() end
      for _, f in ipairs{ "all", "open", "done" } do
        if ui.button(f, { variant = "ghost", selected = self.filter == f }) then self.filter = f end
      end
      ui.spacer()
      if ui.button("Ping", { variant = "outline", icon = "zap" }) then
        if hits ~= nil then hits = hits + 1 elseif count ~= nil then count = count + 1 end
      end
    end)

    local rows = {}
    for _, t in ipairs(self.items) do
      if (self.filter == "all") or (self.filter == "open" and not t.done)
        or (self.filter == "done" and t.done) then table.insert(rows, t) end
    end
    table.sort(rows, function(a, b) return a.star > b.star or (a.star == b.star and a.id > b.id) end)

    if #rows == 0 then
      ui.muted(self.filter == "all" and "nothing yet - type above." or "nothing " .. self.filter .. ".")
    else
      ui.scroll({ height = 200, fade_edges = true, gap = 4 }, function()
        for _, t in ipairs(rows) do
          ui.row({ gap = 8, align = "center" }, function()
            t.done = ui.checkbox("d" .. t.id, t.done)
            ui.text(t.text)
            if t.done then ui.badge("done", { tone = "success" }) end
            ui.spacer()
            t.star = ui.rating("s" .. t.id, t.star, 5)
            if ui.button("", { icon = "trash-2", variant = "ghost", tooltip = "remove" }) then
              for i, x in ipairs(self.items) do if x.id == t.id then table.remove(self.items, i) break end end
            end
          end)
        end
      end)
      local done = 0
      for _, t in ipairs(self.items) do if t.done then done = done + 1 end end
      ui.progress(#self.items > 0 and done / #self.items or 0,
        { tone = "accent", show_value = true, label = string.format("%d of %d", done, #self.items) })
    end

    ui.separator()
    ui.row({ gap = 8, align = "center" }, function()
      ui.muted("shared counter: ")
      ui.badge(tostring(hits or count or 0), { tone = "violet", solid = true })
      if greet and ui.button("greet", { variant = "ghost", size = "sm" }) then
        ui.text(greet("ada"), { font = "mono" })
      end
      ui.kbd("Enter")
      ui.secondary("submits the field")
    end)
  end)
end

widget(graph)
