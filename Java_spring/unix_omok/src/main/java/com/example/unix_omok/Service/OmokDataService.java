package com.example.unix_omok.Service;

import com.example.unix_omok.OmokData;
import com.example.unix_omok.repository.OmokDataRepository;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

@Transactional
public class OmokDataService {

    private final OmokDataRepository omokDataRepository;

    public OmokDataService(OmokDataRepository omokDataRepository) {
        this.omokDataRepository = omokDataRepository;
    }

    public List<OmokData> findList() {
        return omokDataRepository.findAll();
    }
}