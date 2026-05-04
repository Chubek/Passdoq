-- example/scripts/auto_copy_and_mask.lua

-- `lpassdoq` is provided using Sol2, it is built into the system
local passdoq = lpassdoq.init_addon
{
   name = "AutoCopyAndMask",
   author = "Foo",
   maintainer = "Bar",
   version = "1.0.0"
}

local CLIPBOARD_TIMEOUT = 20  -- seconds

-- Mask helper
local function mask(secret)
    if #secret <= 4 then
        return "****"
    end
    return string.rep("*", #secret - 4) .. secret:sub(-4)
end

-- Hook: after secret retrieval
passdoq.on("secret_retrieved", function(entry)
    local value = entry.value

    if not value then
        return
    end

    -- Copy to clipboard
    passdoq.clipboard_set(value)

    -- Mask output
    passdoq.print("Retrieved: " .. mask(value))

    -- Schedule clipboard clear
    passdoq.defer(CLIPBOARD_TIMEOUT, function()
        passdoq.clipboard_clear()
        passdoq.print("[Passdoq] Clipboard cleared")
    end)
end)

-- Optional: restrict plaintext exposure
passdoq.on("before_print_secret", function(entry)
    -- prevent raw printing
    return false
end)

-- Optional: logging
passdoq.print("[auto_copy_and_mask] Loaded")
