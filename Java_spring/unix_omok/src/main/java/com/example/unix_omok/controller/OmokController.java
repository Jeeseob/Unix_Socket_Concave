package com.example.unix_omok.controller;

import com.example.unix_omok.OmokData;
import com.example.unix_omok.Service.OmokDataService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.List;

@Controller
public class OmokController {
    private final OmokDataService omokDataService;

    public OmokController(OmokDataService omokDataService) {
        this.omokDataService = omokDataService;
    }

    @GetMapping("/")
    public String home(Model model) {
        List<OmokData> omokDataList = omokDataService.findList();
        model.addAttribute("omokDataList",omokDataList);
        return "home";
    }

}
